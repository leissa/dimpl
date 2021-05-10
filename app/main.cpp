#include <fstream>
#include <vector>
#include <cctype>
#include <stdexcept>

#include <thorin/util/stream.h>

#include "dimpl/ast.h"
#include "dimpl/bind.h"
#include "dimpl/comp.h"
#include "dimpl/emit.h"
#include "dimpl/parser.h"
#include "dimpl/print.h"

#ifndef NDEBUG
#define LOG_LEVELS "error|warn|info|verbose|debug"
#else
#define LOG_LEVELS "error|warn|info|verbose"
#endif

using namespace dimpl;

static const auto usage =
"Usage: dimpl [options] file...\n"
"\n"
"Options:\n"
"-h, --help                 produce this help message\n"
"    --emit-ast             emit AST of dimpl program\n"
"    --fancy                use fancy output: dimpl's AST dump uses only\n"
"                           parentheses where necessary\n"
"-o, --output               specifies the output module name\n"
"\n"
"Developer options:\n"
"    --log <arg>            specifies log file; use '-' for stdout (default)\n"
"    --log-level {" LOG_LEVELS "}\n"
"                           set log level\n"
#ifndef NDEBUG
"Debugging options:\n"
"-b, --break <args>         trigger a breakpoint when creating a definition of\n"
"                           global id <arg>; may be used multiple times separated\n"
"                           by space or '_'\n"
"    --track-history        track history of names\n"
#endif
"\n"
"Mandatory arguments to long options are mandatory for short options too.\n"
;

std::ostream* open(std::ofstream& stream, const std::string& name) {
    if (name == "-")
        return &std::cout;

    stream.open(name);
    return &stream;
}

template<class... Args> [[noreturn]] void err(const char* fmt, Args&&... args) {
    thorin::StringStream s;
    s.fmt(fmt, std::forward<Args>(args)...);
    throw std::logic_error(s.str());
}

int main(int argc, char** argv) {
    dimpl::Comp comp;

    try {
        if (argc < 1) err("no input files");

        std::vector<std::string> infiles;
        std::string log_name("-"), module_name;

        for (int i = 1; i != argc; ++i) {
            std::string cur_option;

            auto cmp = [&](const char* opt) {
                if (strcmp(argv[i], opt) == 0) {
                    cur_option = opt;
                    return true;
                }
                return false;
            };

            auto get_arg = [&] {
                if (i+1 == argc) err("missing argument for option '{}'", cur_option);
                return std::string(argv[++i]);
            };

            if (cmp("-h") || cmp("--help")) {
                std::cout << usage;
                return EXIT_SUCCESS;
            } else if (cmp("--emit-ast")) {
                comp.emit_ast = true;
            } else if (cmp("--fancy")) {
                comp.fancy = true;
            } else if (cmp("--log")) {
                log_name = get_arg();
            } else if (cmp("--log-level")) {
                auto log_level = get_arg();
                if (false) {}
                else if (log_level == "error"  ) comp.world().set(thorin::LogLevel::Error  );
                else if (log_level == "warn"   ) comp.world().set(thorin::LogLevel::Warn   );
                else if (log_level == "info"   ) comp.world().set(thorin::LogLevel::Info   );
                else if (log_level == "verbose") comp.world().set(thorin::LogLevel::Verbose);
                else if (log_level == "debug"  ) comp.world().set(thorin::LogLevel::Debug  );
                else err("log level must be one of {{" LOG_LEVELS "}}");
            } else if (cmp("-o") || cmp("--output")) {
                module_name = get_arg();
#ifndef NDEBUG
            } else if (cmp("-b") || cmp("--break")) {
                std::string b = get_arg();
                size_t num = 0;
                for (size_t i = 0, e = b.size(); i != e; ++i) {
                    char c = b[i];
                    if (c == '_') {
                        if (num != 0) {
                            comp.world().breakpoint(num);
                            num = 0;
                        }
                    } else if (std::isdigit(c)) {
                        num = num*10 + c - '0';
                    } else {
                        err("invalid breakpoint '{}'", b);
                    }
                }

                if (num != 0)
                    comp.world().breakpoint(num);
            } else if (cmp("--track-history")) {
                comp.world().enable_history();
#endif
            } else if (argv[i][0] == '-') {
                err("unrecognized command line option '{}'", argv[i]);
            } else {
                std::string infile = argv[i];
                auto i = infile.find_last_of('.');
                if (infile.substr(i + 1) != "dimpl")
                    err("input file '{}' does not have '.dimpl' extension", infile);
                auto rest = infile.substr(0, i);
                auto f = rest.find_last_of('/');
                if (f != std::string::npos)
                    rest = rest.substr(f+1);
                if (rest.empty())
                    err("input file '{}' has empty module name", infile);
                if (module_name.empty())
                    module_name = rest;
                infiles.emplace_back(infile);
            }
        }

        std::ofstream log_stream;
        comp.world().set(std::make_shared<thorin::Stream>(*open(log_stream, log_name)));

        if (infiles.empty())
            err("no input files");

        if (infiles.size() != 1) {
            err("at the moment there is only one input file supported");
        }

        auto filename = infiles.front().c_str();
        std::ifstream file(filename, std::ios::binary);
        auto prg = dimpl::parse(comp, file, filename);
        dimpl::Scopes scopes(comp);
        prg->bind(scopes);

        if (comp.emit_ast) {
            Stream s;
            prg->stream(s);
        }

#if 0
        Emitter emitter;
        prg->emit(emitter);
#endif

        return EXIT_SUCCESS;
    } catch (std::exception const& e) {
        comp.err("dimpl: error: {}", e.what());
        return EXIT_FAILURE;
    } catch (...) {
        comp.err("unknown exception");
        return EXIT_FAILURE;
    }
}
