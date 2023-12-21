# gcc -o server server.c -lpthread -curl
# gcc -o test test.c -lpthread -lcurl
# ./server

gcc -o main_server Raspberry.c -lpthread -lcurl -ljansson
# python webcam.py
# python gps.py