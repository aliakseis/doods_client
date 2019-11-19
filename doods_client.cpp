#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "rpc.grpc.pb.h"

class DoodsClient {
public:
    DoodsClient(std::shared_ptr<grpc::Channel> channel)
        : stub_(odrpc::odrpc::NewStub(channel)) {}

    // Assembles the client's payload, sends it and presents the response back
    // from the server.
    std::string SayHello() {
        // Data we are sending to the server.
        google::protobuf::Empty request{};

        // Container for the data we expect from the server.
        odrpc::GetDetectorsResponse reply;

        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.
        grpc::ClientContext context;

        // The actual RPC.
        grpc::Status status = stub_->GetDetectors(&context, request, &reply);

        // Act upon its status.
        if (status.ok()) {
            auto detectors = reply.detectors();
            return detectors[0].name();
        }
        else {
            return "RPC failed";
        }
    }

private:
    std::unique_ptr<odrpc::odrpc::Stub> stub_;
};

int main(int argc, char** argv) {
    // Instantiate the client. It requires a channel, out of which the actual RPCs
    // are created. This channel models a connection to an endpoint (in this case,
    // localhost at port 50051). We indicate that the channel isn't authenticated
    // (use of InsecureChannelCredentials()).
    DoodsClient client(grpc::CreateChannel(
        "localhost:8080", grpc::InsecureChannelCredentials()));
    std::string reply = client.SayHello();
    std::cout << "Greeter received: " << reply << std::endl;

    return 0;
}
