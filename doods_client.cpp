#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <fstream>


#include <grpcpp/grpcpp.h>

#include "rpc.grpc.pb.h"

class DoodsClient {
public:
    DoodsClient(std::shared_ptr<grpc::Channel> channel)
        : stub_(odrpc::odrpc::NewStub(channel)) {}

    // Assembles the client's payload, sends it and presents the response back
    // from the server.
    auto SayHello() {
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
        if (!status.ok()) {
            throw std::runtime_error("RPC failed");
        }

        return reply.detectors();
    }

    auto Recognize(const void* value, size_t size)
    {
        odrpc::DetectRequest request;
        request.set_data(value, size);

        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.
        grpc::ClientContext context;

        odrpc::DetectResponse reply;

        auto status = stub_->Detect(&context, request, &reply);

        // Act upon its status.
        if (!status.ok()) {
            throw std::runtime_error("RPC failed");
        }

        return reply.detections();
    }

private:
    std::unique_ptr<odrpc::odrpc::Stub> stub_;
};

int main(int argc, char** argv) {

    if (argc != 2)
    {
        std::cerr << "Please provide file name.\n";
        return 1;
    }

    try 
    {

        // Instantiate the client. It requires a channel, out of which the actual RPCs
        // are created. This channel models a connection to an endpoint (in this case,
        // localhost at port 50051). We indicate that the channel isn't authenticated
        // (use of InsecureChannelCredentials()).
        DoodsClient client(grpc::CreateChannel(
            "localhost:8080", grpc::InsecureChannelCredentials()));

        auto reply = client.SayHello();
        std::cout << "Greeter received: " << reply[0].name() << std::endl;

        std::ifstream f(argv[1], std::ios::binary);
        std::vector<char> v(std::istreambuf_iterator<char>{f}, {});

        const auto detections = client.Recognize(v.data(), v.size());
        for (auto& detection : detections)
        {
            std::cout << "Left: " << detection.left()
                << " top: " << detection.top()
                << " right: " << detection.right()
                << " bottom: " << detection.bottom()
                << " label: " << detection.label() << '\n';
        }

        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Fatal: " << ex.what() << '\n';
        return 1;
    }
}
