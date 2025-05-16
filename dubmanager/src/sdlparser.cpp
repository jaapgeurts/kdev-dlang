#include <assert.h>

#include <string>
#include <filesystem>
#include <variant>

#include "peglib.h"

#include "sdlparser.h"


using namespace peg;
using namespace std;

void saveSDLProjectFile(shared_ptr<ConfigNode> root, ostream& output) {
    switch (root->type) {
        case NodeType::Nested:
            output << '{';
            for(const auto& child : root->children) {
                saveSDLProjectFile(child,output);
            }
            output << '}';
            break;
        case NodeType::Attribute:
            saveSDLProjectFile(root->children[0],output);
            output << '=';
            saveSDLProjectFile(root->children[1],output);
            break;
        case NodeType::Num64:
            std::visit([&output](const auto& val) { output << val; }, root->value);
            output << 'L';
            break;
        case NodeType::Float32:
            std::visit([&output](const auto& val) { output << val; }, root->value);
            output << 'f';
            break;
        case NodeType::Namespace:
            std::visit([&output](const auto& val) { output << val; }, root->value);
            output << ':';
            break;
        case NodeType::Comment:
        case NodeType::Num32:
        case NodeType::Float64:
        case NodeType::Bool:
        case NodeType::DateTime:
        case NodeType::WhiteSpace:
        case NodeType::Identifier:
            std::visit([&output](const auto& val) { output << val; }, root->value);
            break;
        case NodeType::String:
            output << '"';
            std::visit([&output](const auto& val) { output << val; }, root->value);
            output << '"';
            break;
        case NodeType::RawString:
            output << '\'';
            std::visit([&output](const auto& val) { output << val; }, root->value);
            output << '\'';
            break;
        default:
            for(const auto& child : root->children) {
                saveSDLProjectFile(child,output);
            }
            break;
    }

}


shared_ptr<ConfigNode> readSDLProjectFile(istream& input) {

    auto grammar = R"(
        TagTree         <- ElementLine* Element?
        ElementLine     <- Element EOL
        Element         <- (TagList / Value+ / Spacing)
        TagList         <- Tag (TAGSEP Spacing TagList?)?
        Tag             <- TagName Value* Attribute* Nested?

        Nested          <- Spacing '{'  ElementLine* Spacing '}' Spacing

        TagName         <- Spacing (Namespace ':')? Identifier Spacing
        Namespace       <- Identifier

        Attribute       <- Key '=' Value

        Key             <- Spacing (Namespace ':')? Identifier Spacing

        Value           <- Spacing (StringEscaped / StringRaw / Date / Time / Number / Boolean / Null / Binary) Spacing

        StringEscaped   <-  '"' < ( '\\' . / !'"' . )* > '"'
        StringRaw       <-  '`' < (! '`' . )* > '`'
        Number          <-  Decimal / Float32 / Float64  / Int64 / Int32
        Int32           <- < [0-9]+ >
        Int64           <- < [0-9]+ > [Ll]
        Float32         <- < [0-9]+ '.' [0-9]+ > 'f'
        Float64         <- < [0-9]+ '.' [0-9]+ > 'd'?
        Decimal         <- < [0-9]+ '.' [0-9]+ > 'BD'
        Boolean         <- 'true' / 'false' / 'on' / 'off'
        Null            <- 'null'
        Date            <- < '-'? [0-9]{4} '/' [0-9]{2} '/' [0-9]{2} >
        Time            <- < '-'? ([0-9]+'d' ':')? ([0-9]{2} ':')+ [0-9]{2} ('.' [0-9]{3} ('-' TimeZone)? )? >
        TimeZone        <- < [a-zA-Z0-9/_:+-]+ >
        # base 64 encoded
        Binary          <- '[' < [A-Za-z0-9+/=]* > ']'

        Identifier      <- < [a-zA-Z_] [a-zA-Z0-9_.$-]* >

        Comment         <- BlockComment / LineComment

        LineComment     <- ("//" / "--" / "#") (!EOL !EOF .)*
        BlockComment    <- "/*" < (!"*/" .)* > "*/"

        LineCont        <- '\\' '\n'

        Spacing         <- < ( WS / Comment / LineCont )* >
        TAGSEP          <- ';'
        WS              <- [ \t]+

        EOL             <- '\n' / '\r\n' / '\r'
        EOF             <- !.


    )";

    parser parser;
    parser.set_logger([](size_t line, size_t col, const string& msg, const string &/*rule*/) {
        cerr << line << ":" << col << ": " << msg << "\n";
    });

    auto isParserConstructed = parser.load_grammar(grammar);
    assert(isParserConstructed);
    // assert(static_cast<bool>(parser) == true);


    parser["TagTree"] = [](const SemanticValues &vs) {
        shared_ptr<ConfigNode> root = std::make_shared<ConfigNode>(NodeType::Generic,"TagTree");
        // add all sub values
        for (const auto& v: vs) {
            root->children.push_back(any_cast<shared_ptr<ConfigNode>>(v));
        }
        return root;
    };

    parser["Tag"] = [](const SemanticValues &vs) {
        shared_ptr<ConfigNode> node = std::make_shared<ConfigNode>(NodeType::Generic,"Tag");
        // add all sub values
        // cout << "TAG:\t" << ident << endl;
        for(auto& child : vs) {
            // cout << "\t" << vs.token_to_string() << endl;
            node->children.push_back(any_cast<shared_ptr<ConfigNode>>(child));
        }
        return node;
    };

     parser["Element"] = [](const SemanticValues &vs) {
        shared_ptr<ConfigNode> node = std::make_shared<ConfigNode>(NodeType::Generic,"Element");
        // add all sub values
        // cout << "TAG:\t" << ident << endl;
        for(auto& child : vs) {
            // cout << "\t" << vs.token_to_string() << endl;
            if (!child.has_value())
                continue;
            node->children.push_back(any_cast<shared_ptr<ConfigNode>>(child));
        }
        return node;
    };

    parser["ElementLine"] = [](const SemanticValues &vs) {
        shared_ptr<ConfigNode> node = std::make_shared<ConfigNode>(NodeType::Generic,"Element");
        // add all sub values
        // cout << "TAG:\t" << ident << endl;
        for(auto& child : vs) {
            // cout << "\t" << vs.token_to_string() << endl;
            if (!child.has_value())
                continue;
            node->children.push_back(any_cast<shared_ptr<ConfigNode>>(child));
        }
        return node;
    };

    parser["TagList"] = [](const SemanticValues &vs) {
        shared_ptr<ConfigNode> node = std::make_shared<ConfigNode>(NodeType::Generic,"TagList");
        for(auto& child : vs) {
            // cout << "\t" << vs.token_to_string() << endl;
            node->children.push_back(any_cast<shared_ptr<ConfigNode>>(child));
        }
        return node;
    };

    parser["Nested"] = [](const SemanticValues &vs) {
        shared_ptr<ConfigNode> node = std::make_shared<ConfigNode>(NodeType::Nested,"Nested");
        for(auto& child : vs) {
            // cout << "\t" << vs.token_to_string() << endl;
            node->children.push_back(any_cast<shared_ptr<ConfigNode>>(child));
        }
        return node;
    };

    parser["Attribute"]  = [](const SemanticValues &vs) {
        shared_ptr<ConfigNode> node = std::make_shared<ConfigNode>(NodeType::Attribute,"Attribute");
        for(auto& child : vs) {
            // cout << "\t" << vs.token_to_string() << endl;
            node->children.push_back(any_cast<shared_ptr<ConfigNode>>(child));
        }
        return node;
    };

    parser["Key"] = [](const SemanticValues &vs) {
        shared_ptr<ConfigNode> node = std::make_shared<ConfigNode>(NodeType::Generic,"Key");
        for(auto& child : vs) {
            // cout << "\t" << vs.token_to_string() << endl;
            node->children.push_back(any_cast<shared_ptr<ConfigNode>>(child));
        }
        return node;
    };

    parser["Namespace"] = [](const SemanticValues &vs) {
        // Pull value out of Identifier child node.
        string ident = get<string>(any_cast<shared_ptr<ConfigNode>>(vs[0])->value);
        shared_ptr<ConfigNode> node = std::make_shared<ConfigNode>(NodeType::Namespace,ident);
        return node;
    };


    parser["TagName"] = [](const SemanticValues &vs) {
        shared_ptr<ConfigNode> node = std::make_shared<ConfigNode>(NodeType::Generic,"TagName");
        for(auto& child : vs) {
            // cout << "\t" << vs.token_to_string() << endl;
            node->children.push_back(any_cast<shared_ptr<ConfigNode>>(child));
        }
        // cout << "TAGNAME:\t" << any_cast<string>(vs[0]) << endl;
//        return any_cast<string>(vs[0]);
        return node;
    };

    parser["Identifier"] = [](const SemanticValues &vs) {
        shared_ptr<ConfigNode> node = std::make_shared<ConfigNode>(NodeType::Identifier,vs.token_to_string());
// cout << "IDENT:\t" << ident << endl;
        return node;
    };

    parser["Value"] = [](const SemanticValues &vs) {
        shared_ptr<ConfigNode> node = std::make_shared<ConfigNode>(NodeType::Generic,"Value");
        for(auto& child : vs) {
            // cout << "\t" << vs.token_to_string() << endl;
            node->children.push_back(any_cast<shared_ptr<ConfigNode>>(child));
        }
        return node;
    };

    parser["Int32"] = [](const SemanticValues &vs) {
        shared_ptr<ConfigNode> node = std::make_shared<ConfigNode>(vs.token_to_number<int32_t>());
        return node;
    };
    parser["Int64"] = [](const SemanticValues &vs) {
        shared_ptr<ConfigNode> node = std::make_shared<ConfigNode>(vs.token_to_number<int64_t>());
        return node;
    };
    parser["Float32"] = [](const SemanticValues &vs) {
        shared_ptr<ConfigNode> node = std::make_shared<ConfigNode>(vs.token_to_number<float>());
        return node;
    };
    parser["Float64"] = [](const SemanticValues &vs) {
        shared_ptr<ConfigNode> node = std::make_shared<ConfigNode>(vs.token_to_number<double>());
        return node;
    };
    parser["Decimal"] = [](const SemanticValues &vs) {
        shared_ptr<ConfigNode> node = std::make_shared<ConfigNode>(vs.token_to_number<double>());
        return node;
    };
    parser["Boolean"] = [](const SemanticValues &vs) {
        shared_ptr<ConfigNode> node = std::make_shared<ConfigNode>(NodeType::Bool, vs.token_to_string());
        return node;
    };
    parser["Null"] = [](const SemanticValues &/*vs*/) {
        // an empty node automatically is a null node
        shared_ptr<ConfigNode> node = std::make_shared<ConfigNode>();
        return node;
    };
    parser["DateTime"] = [](const SemanticValues &vs) {
        shared_ptr<ConfigNode> node = std::make_shared<ConfigNode>(NodeType::DateTime,vs.token_to_string());
        return node;
    };
    parser["Date"] = [](const SemanticValues &vs) {
        shared_ptr<ConfigNode> node = std::make_shared<ConfigNode>(NodeType::DateTime,vs.token_to_string());
        return node;
    };
    parser["Time"] = [](const SemanticValues &vs) {
        shared_ptr<ConfigNode> node = std::make_shared<ConfigNode>(NodeType::DateTime,vs.token_to_string());
        return node;
    };
    // parser["Number"] = [](const SemanticValues &vs) {
    //     return vs[0];
    // };
    // parser["Char"] = [](const SemanticValues &vs) {
    //     string res = vs.token_to_string();
    //     if (vs.choice() == 0) {
    //         res = res.substr(1);
    //     }
    //     return res;
    //
    // };
    parser["StringRaw"] = [](const SemanticValues &vs) {
        shared_ptr<ConfigNode> node = std::make_shared<ConfigNode>(NodeType::RawString, vs.token_to_string());
        return node;
    };
    parser["StringEscaped"] = [](const SemanticValues &vs) {
        shared_ptr<ConfigNode> node = std::make_shared<ConfigNode>(vs.token_to_string());
        return node;
    };

    parser["Comment"] = [](const SemanticValues &vs) {
        string text = string(vs.sv());
        // cout << "LINECOMMENT:\t" << text << endl;
        shared_ptr<ConfigNode> node = std::make_shared<ConfigNode>(NodeType::Comment, text);
        return node;
    };

    parser["Spacing"] = [](const SemanticValues &vs) {
        shared_ptr<ConfigNode> node = std::make_shared<ConfigNode>(NodeType::WhiteSpace, string(vs.sv()));
        // cout << "SPACING: '" << string(vs.sv()) << "'" << endl;
        return node;
    };

     parser["EOL"] = [](const SemanticValues &vs) {
        shared_ptr<ConfigNode> node = std::make_shared<ConfigNode>(NodeType::WhiteSpace,string(vs.sv()));
        // cout << "EOL: '" << string(vs.sv()) << "'" << endl;
        return node;
    };


    std::ostringstream buffer;
    buffer << input.rdbuf();
    string text = buffer.str();

    shared_ptr<ConfigNode> ast;
    parser.parse(text,ast);

    return ast;

}

