#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <winsock2.h>

#define DEFAULT_NUM_THREADS 5
#define DEFAULT_TCP_PORT 8080
#define DEFAULT_PACKET_SIZE 5120
#define DEFAULT_PACKET_COUNT 50

// Global variables
static int num_threads = DEFAULT_NUM_THREADS;
static int tcp_port = DEFAULT_TCP_PORT;
static int packet_size = DEFAULT_PACKET_SIZE;
static int packet_count = DEFAULT_PACKET_COUNT;

// Function to fill buffer with random data
void fill_buffer(char* buffer, size_t size) {
    for (size_t i = 0; i < size; i++) {
        buffer[i] = '0' + (rand() % 10);
    }
}

// Function to handle the client socket communication
void handle_client_socket(unsigned long long client_socket, char* buffer) {
    for (int i = 0; i < packet_count; i++) {
        // Send data to the server
        size_t bytes_sent = send(client_socket, buffer, packet_size, 0);
        if (bytes_sent < 0) {
            perror("send error");
            return;
        }

        // Receive data from the server
        size_t bytes_received = recv(client_socket, buffer, packet_size, 0);
        if (bytes_received < 0) {
            perror("recv error");
            return;
        }
    }
}

// Thread function for measuring performance
void* measure_performance(void* arg) {
    int thread_id = *(int*)arg;

    // Allocate buffer for packet data
    char* buffer = (char*)malloc(packet_size);
    if (!buffer) {
        perror("malloc failed");
        pthread_exit(NULL);
    }

    // Fill the buffer with random data
    fill_buffer(buffer, packet_size);

    // Create a TCP socket
    unsigned long long client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("socket creation failed");
        free(buffer);
        pthread_exit(NULL);
    }

    // Configure server address
    struct sockaddr_in tcp_servaddr;
    tcp_servaddr.sin_family = AF_INET;
    tcp_servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    tcp_servaddr.sin_port = htons(tcp_port);

    // Connect to the server
    if (connect(client_socket, (struct sockaddr*)&tcp_servaddr, sizeof(tcp_servaddr)) < 0) {
        printf("Thread %d: Connect failed with error: %d\n", thread_id, WSAGetLastError());
        closesocket(client_socket);
        free(buffer);
        pthread_exit(NULL);
    }

    printf("Thread %d: Connected to TCP server on port %d.\n", thread_id, tcp_port);

    // Handle client-server communication
    handle_client_socket(client_socket, buffer);

    // Cleanup
    free(buffer);
    closesocket(client_socket);

    printf("Thread %d: Connection closed.\n", thread_id);
    pthread_exit(NULL);
}

// Initialize Winsock library
void initialize_winsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed with error: %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
}

// Main function
int main(void) {
    // Initialize Winsock
    initialize_winsock();

    // Seed the random number generator
    srand((unsigned int)time(NULL));

    // Create an array for thread IDs
    pthread_t threads[num_threads];
    int thread_ids[num_threads];

    // Create threads
    for (int i = 0; i < num_threads; i++) {
        thread_ids[i] = i;
        int res = pthread_create(&threads[i], NULL, measure_performance, (void*)&thread_ids[i]);
        if (res != 0) {
            fprintf(stderr, "Error creating thread %d\n", i);
            exit(EXIT_FAILURE);
        }
    }

    // Join threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Cleanup Winsock
    WSACleanup();

    return 0;
}
