#include <exception>
#include <iostream>

#include <fmt/core.h>
#include <httplib.h>
#include <argparse/argparse.hpp>
#include <nlohmann/json.hpp>

namespace {

std::string make_prompt(const std::string& from, const std::string& to,
                        const std::string& text) {
    return fmt::format(
        "<|plamo:op|>dataset\n"
        "translation\n"
        "<|plamo:op|>input lang={}\n"
        "{}\n"
        "<|plamo:op|>output lang={}\n",
        from, text, to);
}

}  // namespace

int main(int argc, char* argv[]) {
    argparse::ArgumentParser program("translate");
    program.add_description("Translate by plamo-2-translate.");

    program.add_argument("--from")
        .help("source language")
        .metavar("language")
        .required();
    program.add_argument("--to")
        .help("target language")
        .metavar("language")
        .required();

    auto& text_group = program.add_mutually_exclusive_group(true);
    text_group.add_argument("--text").metavar("text").help(
        "Translate this literal string.");
    text_group.add_argument("--stdio")
        .implicit_value(true)
        .default_value(false)
        .help("Read the text to translate from stdio.");
    text_group.add_argument("--editor")
        .metavar("command")
        .help("Read the text to translate from editor.");

    program.add_argument("--host")
        .metavar("host")
        .default_value("localhost")
        .help("hostname or IP address.");
    program.add_argument("--port").metavar("port").default_value("11434").help(
        "port number");
    program.add_argument("--model")
        .metavar("model")
        .default_value("mitmul/plamo-2-translate:Q4_K_S")
        .help("plamo-2-translate model name");
    program.add_argument("--context")
        .metavar("context")
        .default_value(4096)
        .scan<'i', int>()
        .help("context length");

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n" << program;
        return 1;
    }

    auto from = program.get<std::string>("--from");
    auto to = program.get<std::string>("--to");

    std::string text;
    if (program.is_used("--text")) {
        text = program.get<std::string>("--text");
    } else if (program.is_used("--stdio")) {
        std::ostringstream oss;
        oss << std::cin.rdbuf();
        text = oss.str();
    }

    auto host = program.get<std::string>("--host");
    auto port = program.get<std::string>("--port");
    auto model = program.get<std::string>("--model");
    auto context = program.get<int>("--context");

    httplib::Client cli(fmt::format("http://{}:{}", host, port));

    nlohmann::json req = {
        {"model", model},
        {"prompt", make_prompt(from, to, text)},
        {"stream", true},
        {"options",
         {
             {"temperature", 0.0},
             {"num_ctx", context},

         }},
    };

    std::string buffer;

    auto res = cli.Post(
        "/api/generate", httplib::Headers(), req.dump(), "application/json",
        [&](const char* data, size_t data_len) {
            buffer.append(data, data_len);

            size_t pos = 0;
            while (true) {
                size_t newline = buffer.find('\n', pos);
                if (newline == std::string::npos) {
                    break;
                }

                std::string line = buffer.substr(pos, newline - pos);

                if (!line.empty()) {
                    try {
                        auto json = nlohmann::json::parse(line);

                        if (json.contains("response")) {
                            std::cout << json["response"].get<std::string>();
                            std::cout.flush();
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "JSON parse error: " << e.what() << "\n";
                        std::cerr << "line: " << line << "\n";
                    }
                }

                pos = newline + 1;
            }

            if (pos > 0) {
                buffer.erase(0, pos);
            }

            return true;
        },
        nullptr);

    if (!res) {
        std::cerr << "Request failed: " << httplib::to_string(res.error())
                  << "\n";
        return 1;
    }

    return 0;
}
