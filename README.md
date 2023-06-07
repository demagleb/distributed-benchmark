## Code Performance Clustering

This project is a system consisting of multiple servers clustered together to execute and measure the performance of arbitrary code on multiple machines simultaneously. Additionally, the project allows connecting to a master server, uploading an executable file, and receiving the execution results on all worker nodes with detailed statistics.

### Project Components

The project consists of three main components:

1. **Master Server**: Responsible for cluster organization and receiving work from clients. It creates listening TCP sockets to register worker nodes and accept tasks from clients. After registration and code loading on the worker nodes, the master server waits for completion of work on all nodes and sends the execution results back to the client.

2. **Worker Nodes**: Represent the worker nodes in the cluster, which register with the master server and execute assigned tasks. During registration, each node sends information about its resources (such as CPU, core count, and memory). Then, the nodes wait for tasks from the master server. Upon receiving a task in the form of an executable file, the nodes save it, apply the seccomp security mechanism, and execute the code. Upon task completion, the results are sent to the master server.

3. **Client**: It provides a user interface that allows uploading a code file to the master server and receiving task execution results. The client needs to specify the file name and establish a connection to the master server to send the file and retrieve the results.

### Project Execution

To work with the project, it is necessary to pre-install the **libseccomp** library. Each component is launched as follows:

- **Client**: The client is launched using the command `client FILENAME NODE SERVICE`, where `FILENAME` is the name of the code file, `NODE` is the address of the master server node, and `SERVICE` is the port of the master server.

- **Master Server**: The master server is launched using the command `server SERVICE PORT_FOR_WORKERS PORT_FOR_CLIENTS`, where `SERVICE` is the service port of the master server, `PORT_FOR_WORKERS` is the port for registering worker nodes, and `PORT_FOR_CLIENTS` is the port for receiving work from clients.

- **Worker Nodes**: The worker nodes are launched using the command `worker NODE SERVICE`, where `NODE` is the IPv4 address of the master server and `SERVICE` is the port of the master server.

Each component performs its tasks, ensuring interaction and code performance measurement in the cluster.