#!/env/bin/bash

# Test construct_message() and get_payload_size() functions
python3 ./test_messages.py

# Test the secure_recv() and secure_sent() functions
python3 ./test_2.py

# Test single-threaded server and client
python3 ./test_3.py

# Test multithreaded-server and clients
python3 ./test_4.py

# Test multithreaded-server and clients
python3 ./test_5.py

# Test multithreaded-server and clients
python3 ./test_6.py

# Test repeated requests for connections
python3 ./test_7.py

echo "Finished tests .."
