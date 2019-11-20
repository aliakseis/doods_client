#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>


#include <grpcpp/grpcpp.h>

#include "rpc.grpc.pb.h"

#include <boost/gil/extension/io/jpeg.hpp>


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

namespace gil = boost::gil;

int main(int argc, char** argv) {

    if (argc < 2)
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

        if (argc > 2)
        {
            gil::rgb8_image_t img;
            gil::read_image(argv[1], img, gil::jpeg_tag());

            auto img_view = view(img);

            const auto width = img_view.width();
            const auto height = img_view.height();

            const float x_max = width - 1;
            const float y_max = height - 1;

            const gil::rgb8_pixel_t px{ 0, 255, 0 };

            for (auto& detection : detections)
            {
                const int left = std::min(detection.left() * width, x_max);
                const int right = std::min(detection.right() * width, x_max);

                const int top = std::min(detection.top() * height, y_max);
                const int bottom = std::min(detection.bottom() * height, y_max);

                for (int i = left; i <= right; ++i)
                {
                    *img_view.at(i, top) = px;
                    *img_view.at(i, bottom) = px;
                }

                for (int i = top; i <= bottom; ++i)
                {
                    *img_view.at(left, i) = px;
                    *img_view.at(right, i) = px;
                }
            }

            write_view(argv[2], img_view, gil::jpeg_tag());
        }

        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Fatal: " << ex.what() << '\n';
        return 1;
    }
}
