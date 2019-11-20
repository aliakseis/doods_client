#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <map>


#include <grpcpp/grpcpp.h>

#include "rpc.grpc.pb.h"

#include <boost/gil/extension/io/jpeg.hpp>

namespace {

const std::map<char, unsigned long long> font{
    { ' ', 0b000000000000000000000000000000000000000000000000 },
    { '!', 0b000000000000000011110110000000000000000000000000 },
    { '"', 0b000000001100000000000000110000000000000000000000 },
    { '#', 0b001010001111111000101000111111100010100000000000 },
    { '$', 0b001001000101010011111110010101000100100000000000 },
    { '%', 0b000000100100110000010000011000001000010000000000 },
    { '&', 0b000111001110001010110010110011000000001000000000 },
    { '\\',0b100000000110000000010000000011000000001000000000 },
    { '(', 0b000000000000000001111100100000100000000000000000 },
    { ')', 0b000000001000001001111100000000000000000000000000 },
    { '*', 0b010010000011000011100000001100000100100000000000 },
    { '+', 0b000100000001000001111100000100000001000000000000 },
    { ',', 0b000000000000000000000110000000000000000000000000 },
    { '-', 0b000100000001000000010000000100000001000000000000 },
    { '.', 0b000000000000000000000100000000000000000000000000 },
    { '/', 0b000000100000110000010000011000001000000000000000 },
    { '0', 0b011111001000001010000010100000100111110000000000 },
    { '1', 0b000000001000001011111110000000100000000000000000 },
    { '2', 0b010011101001001010010010100100100111001000000000 },
    { '3', 0b010001001000001010000010100100100110110000000000 },
    { '4', 0b111100000001000000010000000100001111111000000000 },
    { '5', 0b111001001001001010010010100100101001110000000000 },
    { '6', 0b011111001001001010010010100100101000110000000000 },
    { '7', 0b100000001000000010000110100110001110000000000000 },
    { '8', 0b011011001001001010010010100100100110110000000000 },
    { '9', 0b011000001001000010010000100100000111111000000000 },
    { ':', 0b000000000000000001000100000000000000000000000000 },
    { ';', 0b000000000000000001000110000000000000000000000000 },
    { '<', 0b000000000001000000101000010001000000000000000000 },
    { '=', 0b001010000010100000101000001010000000000000000000 },
    { '>', 0b000000000100010000101000000100000000000000000000 },
    { '?', 0b010000001000000010001010100100000110000000000000 },
    { '@', 0b011111001000001010111010101010100111001000000000 },
    { 'A', 0b011111101001000010010000100100000111111000000000 },
    { 'B', 0b111111101001001010010010100100100110110000000000 },
    { 'C', 0b011111001000001010000010100000100100010000000000 },
    { 'D', 0b111111101000001010000010100000100111110000000000 },
    { 'E', 0b111111101001001010010010100100101000001000000000 },
    { 'F', 0b111111101001000010010000100100001000000000000000 },
    { 'G', 0b011111001000001010000010100010100100110000000000 },
    { 'H', 0b111111100001000000010000000100001111111000000000 },
    { 'I', 0b100000101000001011111110100000101000001000000000 },
    { 'J', 0b000011000000001000000010000000101111110000000000 },
    { 'K', 0b111111100001000000010000001010001100011000000000 },
    { 'L', 0b111111100000001000000010000000100000001000000000 },
    { 'M', 0b111111101000000001100000100000001111111000000000 },
    { 'N', 0b111111100100000000100000000100001111111000000000 },
    { 'O', 0b011111001000001010000010100000100111110000000000 },
    { 'P', 0b111111101001000010010000100100001111000000000000 },
    { 'Q', 0b011111001000001010001010100001000111101000000000 },
    { 'R', 0b111111101001000010010000100110001111011000000000 },
    { 'S', 0b011000101001001010010010100100101000110000000000 },
    { 'T', 0b100000001000000011111110100000001000000000000000 },
    { 'U', 0b111111000000001000000010000000101111110000000000 },
    { 'V', 0b111110000000010000000010000001001111100000000000 },
    { 'W', 0b111111100000001000001100000000101111111000000000 },
    { 'X', 0b110001100010100000010000001010001100011000000000 },
    { 'Y', 0b110000000010000000011110001000001100000000000000 },
    { 'Z', 0b100001101000101010010010101000101100001000000000 },
    { '[', 0b000000001111111010000010100000100000000000000000 },
    { '\'',0b100000000110000000010000000011000000001000000000 },
    { ']', 0b000000001000001010000010111111100000000000000000 },
    { '^', 0b001000000100000010000000010000000010000000000000 },
    { '_', 0b000000100000001000000010000000100000001000000000 },
    { '`', 0b000000001000000001000000000000000000000000000000 },
    { 'a', 0b000001000010101000101010001010100001111000000000 },
    { 'b', 0b111111100001001000010010000100100000110000000000 },
    { 'c', 0b000111000010001000100010001000100001010000000000 },
    { 'd', 0b000011000001001000010010000100101111111000000000 },
    { 'e', 0b000111000010101000101010001010100001101000000000 },
    { 'f', 0b000100000111111010010000100100000000000000000000 },
    { 'g', 0b001100100100100101001001010010010011111000000000 },
    { 'h', 0b111111100001000000010000000100000000111000000000 },
    { 'i', 0b000000000000000001011110000000000000000000000000 },
    { 'j', 0b000000100000000100000001010111100000000000000000 },
    { 'k', 0b111111100000100000010100001000100000000000000000 },
    { 'l', 0b000000000000000011111110000000000000000000000000 },
    { 'm', 0b000111100001000000001000000100000001111000000000 },
    { 'n', 0b001111100001000000010000000100000001111000000000 },
    { 'o', 0b000111000010001000100010001000100001110000000000 },
    { 'p', 0b001111110010010000100100001001000001100000000000 },
    { 'q', 0b000110000010010000100100001001000011111100000000 },
    { 'r', 0b000000000011111000010000000100000000000000000000 },
    { 's', 0b000000000001001000101010001010100010010000000000 },
    { 't', 0b000000000010000011111110001000000000000000000000 },
    { 'u', 0b000111000000001000000010000000100001110000000000 },
    { 'v', 0b000110000000010000000010000001000001100000000000 },
    { 'w', 0b000111100000001000000100000000100001111000000000 },
    { 'x', 0b001000100001010000001000000101000010001000000000 },
    { 'y', 0b001100000000100000000111000010000011000000000000 },
    { 'z', 0b010001100100101001010010011000100000000000000000 },
    { '{', 0b000000000000000001101100100100100000000000000000 },
    { '|', 0b000000000000000011111110000000000000000000000000 },
    { '}', 0b000000000000000010010010011011000000000000000000 },
    { '~', 0b000100000010000000010000000010000001000000000000 },
};

void draw_point(boost::gil::rgb8_view_t& view, int x, int y, const boost::gil::rgb8_pixel_t& px) {
    const auto width = view.width();
    const auto height = view.height();
    if (x >= 0 && x < width && y >= 0 && y < height) {
        *view.at(x, y) = px;
    }
}

void draw_char(boost::gil::rgb8_view_t& view, int x, int y, char c, const boost::gil::rgb8_pixel_t& px) {
    auto it = font.find(c);
    if (it != font.end())
    {
        const auto letter = it->second;
        unsigned long long n = 1ull << 47;
        for (int i = 0; i < 6; i++) {
            for (int j = 0; j < 8; j++) {
                if (letter & n) {
                    draw_point(view, x + i, y + j, px);
                }
                n >>= 1;
            }
        }
    }
}

void draw_string(boost::gil::rgb8_view_t& view, int x, int y, const std::string& text, const boost::gil::rgb8_pixel_t& px) {
    for (unsigned int i = 0; i < text.length(); i++) {
        draw_char(view, x + 6 * i, y, text[i], px);
    }

}


} // namespace


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

                draw_string(img_view, left + 2, top + 2, detection.label(), px);
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
