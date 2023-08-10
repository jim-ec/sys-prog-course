#include <iostream>
#include <string_view>
#include <fstream>
#include <filesystem>
#include <vector>
#include <optional>
#include <cstring>

#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/mount.h>


namespace fs = std::filesystem;


int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << "  [build-directory] [command] [arguments...]" << std::endl;
        exit(EXIT_FAILURE);
    }

    fs::path inputBuild = argv[1];
    if (!fs::exists(inputBuild)) {
        std::cerr << "Build directory `" << inputBuild << "` does not exist" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::ifstream envVars = inputBuild / "env-vars";
    std::string line;
    fs::path shell;
    while (std::getline(envVars, line)) {
        std::string_view shellVarPrefix = "declare -x SHELL=";
        if (line.find(shellVarPrefix) != std::string::npos) {
            shell = line.substr(shellVarPrefix.length() + 1, line.length() - shellVarPrefix.length() - 2);
        }
    }
    if (shell.empty()) {
        std::cerr << "Could not find a variable specifying the shell executable" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::clog << "Using shell: " << shell << std::endl;

    uid_t outerUID = getuid();
    gid_t outerGID = getgid();

    std::string rootName = "/tmp/nix-build-shell-XXXXXX";
    if (mkdtemp(rootName.data()) == nullptr) {
        perror("mkdtemp");
        exit(EXIT_FAILURE);
    }
    fs::path root = rootName;
    std::clog << "Root at: " << root << std::endl;

    // Copy given build directory to the our root directory
    fs::path build = root / "build";
    fs::create_directory(build);

    // A simple fs::copy(.., fs::recursive) does not work, because the input build tree may contain
    // files which are exclusivly accessible to the NIX build user.
    // A recursive copy would fail in that case, instead of skipping the private file.
    for (const auto& entry : fs::recursive_directory_iterator(inputBuild)) {
        const auto target = build / entry.path().lexically_relative(inputBuild);
        if (fs::status(entry.path()).type() == fs::file_type::directory) {
            fs::create_directory(target);
            fs::permissions(target, entry.status().permissions());
        }
        else if ((fs::status(entry.path()).permissions() & fs::perms::others_read) != fs::perms::none) {
            fs::copy(entry.path(), target, fs::copy_options::copy_symlinks);
        }
    }

    // Create namespace
    if (0 != unshare(CLONE_NEWUSER | CLONE_NEWUTS | CLONE_NEWNET | CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWIPC)) {
        perror("Cannot unshare");
        exit(EXIT_FAILURE);
    }

    // Fork so we actually are in the new PID namespace
    if (auto pid = fork(); pid > 0) {
        int result;
        waitpid(pid, &result, 0);
        exit(result);
    }

    // Write UID mapping
    std::ofstream("/proc/self/uid_map") << 1000 << ' ' << outerUID << ' ' << 1 << std::endl;

    // Write GID mapping
    std::ofstream("/proc/self/setgroups") << "deny" << std::endl;
    std::ofstream("/proc/self/gid_map") << 100 << ' ' << outerGID << ' ' << 1 << std::endl;

    // Write host name and domain name
    std::string_view hostName = "localhost";
    if (0 != sethostname(hostName.data(), hostName.length())) {
        perror("sethostname");
        exit(EXIT_FAILURE);
    }
    std::string_view domainName = "(none)";
    if (0 != setdomainname(domainName.data(), domainName.length())) {
        perror("setdomainname");
        exit(EXIT_FAILURE);
    }

    // Write loopback devices
    {
        int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
        if (!fd) {
            perror("cannot open IP socket");
            exit(EXIT_FAILURE);
        }
        struct ifreq ifr;
        strcpy(ifr.ifr_name, "lo");
        ifr.ifr_flags = IFF_UP | IFF_LOOPBACK | IFF_RUNNING;
        if (ioctl(fd, SIOCSIFFLAGS, &ifr) != 0) {
            perror("cannot set loopback interface flags");
            exit(EXIT_FAILURE);
        }
        close(fd);
    }

    auto bindMount = [&] (fs::path path, std::optional<fs::path> source = {}) {
        auto target = root / path.lexically_relative("/");
        fs::create_directories(target.parent_path());
        if (fs::is_directory(path)) {
            fs::create_directory(target);
        }
        else {
            std::ofstream{target};
        }
        if (mount(source.has_value() ? source.value().c_str() : path.c_str(), target.c_str(), "", MS_REC | MS_BIND, nullptr) != 0) {
            std::cerr << "Cannot mount " << path << std::endl;
            perror("");
            exit(EXIT_FAILURE);
        }
    };

    // Mount NIX directory
    bindMount("/nix");

    // Mount /bin/sh for `system()` compatibility
    bindMount("/bin/sh", shell);

    // Mount common /dev files
    bindMount("/dev/full");
    bindMount("/dev/kvm");
    bindMount("/dev/null");
    bindMount("/dev/random");
    bindMount("/dev/tty");
    bindMount("/dev/urandom");
    bindMount("/dev/zero");
    bindMount("/dev/ptmx");

    // Required to control the connected terminal
    bindMount("/dev/pts");

    // Mount /dev/shm as tempfs
    auto shm = root / "dev/shm";
    fs::create_directories(shm);
    if (mount("", shm.c_str(), "tmpfs", 0, nullptr) != 0) {
        std::cerr << "Cannot mount /dev/shm" << std::endl;
        perror("");
        exit(EXIT_FAILURE);
    }

    // Create /tmp
    fs::create_directories(root / "tmp");
    fs::permissions(root / "tmp", fs::perms::all | fs::perms::sticky_bit);

    // Mount proc
    auto proc = root / "proc";
    fs::create_directories(proc);
    if (mount("", proc.c_str(), "proc", 0, nullptr) != 0) {
        std::cerr << "Cannot mount /proc" << std::endl;
        perror("");
        exit(EXIT_FAILURE);
    }

    // Create /etc files
    auto etc = root / "etc";
    fs::create_directories(etc);
    std::ofstream(etc / "group") << "root:x:0:\n" << "nixbld:!:100:\n" << "nogroup:x:65534:\n";
    std::ofstream(etc / "passwd") << "root:x:0:0:Nix build user:/build:/noshell\n"
        << "nixbld:x:1000:100:Nix build user:/build:/noshell\n"
        << "nobody:x:65534:65534:Nobody:/:/noshell\n";
    std::ofstream(etc / "hosts") << "127.0.0.1 localhost\n" << "::1 localhost\n";

    // Change root and current directory
    if (0 != chdir(root.c_str())) {
        perror("chdir");
    }
    if (0 != chroot(".")) {
        perror("chroot");
    }

    // Create standard symbolic links
    fs::create_symlink("/proc/self/fd", "/dev/fd");
    fs::create_symlink("/proc/self/fd/0", "/dev/stdin");
    fs::create_symlink("/proc/self/fd/1", "/dev/stdout");
    fs::create_symlink("/proc/self/fd/2", "/dev/stderr");

    // Make root private
    if (mount("/", "/", "", MS_PRIVATE | MS_REC | MS_BIND, 0) == -1) {
        perror("cannot make / private");
        exit(EXIT_FAILURE);
    }

    // Execute command line
    std::vector<char*> commandLine;
    commandLine.push_back(strdup(shell.c_str()));
    commandLine.push_back(strdup("-c"));
    commandLine.push_back(strdup("source /build/env-vars; exec \"$@\""));
    commandLine.push_back(strdup("--"));
    for (int i = 2; i < argc; ++i) {
        commandLine.push_back(strdup(argv[i]));
    }
    commandLine.push_back(nullptr);
    execv(commandLine[0], &commandLine[0]);
    std::cerr << "Could not execute the given command line" << std::endl;
    exit(EXIT_FAILURE);
}
