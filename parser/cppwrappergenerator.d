module cppwrappergenerator;

import std.stdio;
import std.uni;
import std.string;
import std.algorithm.searching;

import dparse.lexer;
import dparse.parser;
import dparse.ast;
import dparse.rollback_allocator;

private enum keywords = [
        "abstract", "alias", "align", "asm", "assert", "auto", "body", "bool",
        "break", "byte", "case", "cast", "catch", "cdouble", "cent", "cfloat",
        "char", "class", "const", "continue", "creal", "dchar", "debug", "default",
        "delegate", "delete", "deprecated", "do", "double", "else", "enum",
        "export", "extern", "false", "final", "finally", "float", "for", "foreach",
        "foreach_reverse", "function", "goto", "idouble", "if", "ifloat",
        "immutable", "import", "in", "inout", "int", "interface", "invariant",
        "ireal", "is", "lazy", "long", "macro", "mixin", "module", "new",
        "nothrow", "null", "out", "override", "package", "pragma", "private",
        "protected", "public", "pure", "real", "ref", "register", "return",
        "scope", "shared", "short", "static", "struct", "super", "switch",
        "synchronized", "template", "this", "throw", "true", "try", "typedef",
        "typeid", "typeof", "ubyte", "ucent", "uint", "ulong", "union", "unittest",
        "ushort", "version", "void", "volatile", "wchar", "while", "with",
        "__DATE__", "__EOF__", "__FILE__", "__FUNCTION__", "__gshared",
        "__LINE__", "__MODULE__", "__parameters", "__PRETTY_FUNCTION__",
        "__TIME__", "__TIMESTAMP__", "__traits", "__vector", "__VENDOR__",
        "__VERSION__"
    ];

struct ClassVariable {
    string type, name, args;
    bool isArray;
    bool isClass;
    bool isEnum;
}

struct Class {
    string name;
    bool isExpression;
}

ClassVariable[][Class] classes;

Declaration[string] templatedClasses;

struct Alias {
    string name;
    string templatename;
    string templateargs;
}

Alias[string] aliasses;

struct Enum {
    string name;
    string[] values;
}

Enum[] enums;

ClassVariable[] classvars;
string[string] allclasses;

string escapeName(string name) {
    import std.algorithm;

    return keywords.canFind(name) ? name ~ "_" : name;
}

string escapeAndLowerName(string name) {
    return escapeName(format("%s%s", name[0].toLower(), name[1 .. $]));
}

string getString(const Token token) {
    return token.text;
}

string getString(const IdentifierOrTemplateChain identifierOrTemplateChain) {
    string str;
    foreach (i, instance; identifierOrTemplateChain.identifiersOrTemplateInstances) {
        str ~= getString(instance.identifier);
        if (i + 1 < identifierOrTemplateChain.identifiersOrTemplateInstances.length)
            str ~= ".";
    }
    return str;
}

string getString(const Symbol symbol) {
    auto str = getString(symbol.identifierOrTemplateChain);
    if (symbol.dot)
        str = format(".%s", str);
    return str;
}

private MixinTemplateDeclaration[string] mixins;

// rewrite using visitor pattern
void main(string[] args) {

    LexerConfig config;
    config.fileName = args[1];

    RollbackAllocator allocator;

    auto file = File(args[1], "r");
    auto source = new ubyte[](file.size());
    file.rawRead(source);
    auto tokens = getTokensForParser(source, config,
            new StringCache(StringCache.defaultBucketCount));

    auto mod = parseModule(tokens, config.fileName, &allocator);

    // old(mod,args);

    mod.accept(new pass1Visitor());

    writeHeader();

    writeExtras();

    mod.accept(new WrapperGenVisitor(WrapperGenVisitor.GenType.GenInterfaces));

    mod.accept(new WrapperGenVisitor(WrapperGenVisitor.GenType.GenClasses));

}

void writeHeader() {

    // generate D Wrapper
    writeln("module astWrapper;");
    writeln();
    writeln("import std.string;");
    writeln();
    writeln("import dparse.lexer;");
    writeln("import dparse.parser;");
    writeln("import dparse.ast;");
    writeln();

    // writeln("import std.variant : Algebraic;");
    // writeln(); 

    writeln("template ContextMethods()");
    writeln("{");
    writeln("	extern(C++) void* getContext()");
    writeln("	{");
    writeln("		return context;");
    writeln("	}");
    writeln("	extern(C++) void setContext(void* context)");
    writeln("	{");
    writeln("		this.context = context;");
    writeln("	}");
    writeln("	__gshared void* context;");
    writeln("}");
    writeln();

    // Enum kind
    writeln("enum Kind");
    writeln("{");
    foreach (cv; allclasses) {
        writefln("\t%s,", escapeAndLowerName(cv));
    }
    writeln("}");
    writeln();

    //INode.
    writeln("extern(C++) interface INode");
    writeln("{");
    writeln("	Kind getKind();");
    writeln("	void* getContext();");
    writeln("	void setContext(void* context);");
    writeln("}");
    writeln();
}

void writeExtras() {

    // Token
    writeln("extern(C++) interface IToken : INode {}");
    /*        writeln(
                newClassType.name = "Token";
        ClassVariable cv;
        
        cv.type = "string";
        cv.name = "text";
        classes[newClassType] ~= cv;
        cv.type = "size_t";
        cv.name = "line";
        classes[newClassType] ~= cv;
        cv.type = "size_t";
        cv.name = "column";
        classes[newClassType] ~= cv;*/
}

class pass1Visitor : ASTVisitor {

    alias visit = ASTVisitor.visit;

    override void visit(const MixinTemplateDeclaration mtd) {
        mixins[getString(mtd.templateDeclaration.name)] = cast(MixinTemplateDeclaration) mtd;
    }

    override void visit(const ClassDeclaration cd) {
        // collect all class names
        if (cd.name.text == "ASTVisitor")
            return;
        allclasses[cd.name.text] = cd.name.text;
    }

    override void visit(const Unittest ut) {
        return;
    }

}

class WrapperGenVisitor : ASTVisitor {

    enum GenType {
        GenInterfaces,
        GenClasses
    }

    GenType genType;

    alias visit = ASTVisitor.visit;

    this(GenType genType) {
        this.genType = genType;
    }

    override void visit(const ModuleDeclaration md) {

        super.visit(md);
    }

    override void visit(const ClassDeclaration cd) {

        if (cd.name.text == "ASTVisitor")
            return;

        if (genType == GenType.GenInterfaces) {
            writefln("extern(C++) interface I%s : INode", cd.name.text);
            writeln("{");
        }
        else {
            writefln("extern(C++) class C%s : INode", cd.name.text);
            writeln("{");
            writeln("public: //Methods.");
            writefln("\tthis(const %s dclass)", cd.name.text);
            writeln("\t{");
            writeln("\t\tthis.dclass = dclass;");
            writeln("\t}");
            writeln("\t");
            writeln("\textern(C++) Kind getKind()");
            writeln("\t{");
            writefln("\t\treturn Kind.%s;", escapeAndLowerName(cd.name.text));
            writeln("\t}");
            writeln("\t");
            writeln("\tmixin ContextMethods;");
            writeln();
        }
        classvars.length = 0;
        super.visit(cd);

        foreach (cv; classvars) {
            if (cv.type == "string")
                cv.type = "const(char)*";

            string typeorname = cv.type;
            if (cv.type in allclasses)
                typeorname = "I" ~ cv.type;

            if (cv.isArray) {
                writefln("\tsize_t num%s%s();", cv.name[0].toUpper(),
                        cv.name[1 .. $].replace("_", ""));
                string name = format("%s%s", cv.name[0].toUpper(),
                        cv.name[1 .. $].replace("_", ""));
                if (name.endsWith("ses"))
                    name = name[0 .. $ - 3];
                else if (name.endsWith("xes"))
                    name = name[0 .. $ - 2];
                else if (name.endsWith("s"))
                    name = name[0 .. $ - 1];
                writef("\t%s get%s(size_t index", typeorname, name);
            }
            else {
                writef("\t%s get%s%s(", typeorname, cv.name[0].toUpper,
                        cv.name[1 .. $].replace("_", ""));
            }

            if (genType == GenType.GenClasses) {
                writeln(")");
                writeln("\t{");
                writefln("\t\tif(!%s%s)", escapeName(cv.name), cv.type == "Token"
                        || cv.type == "string" ? "" : format(" && dclass.%s", cv.name));
                if (cv.type == "IdType")
                    writefln("\t\t\t%s%s = toStringz(str(dclass.%s%s));",
                            escapeName(cv.name), "", cv.name, "");
                else if (cv.type == "string")
                    writefln("\t\t\t%s%s = toStringz(dclass.%s%s);",
                            escapeName(cv.name), "", cv.name, "");
                else if (cv.type[0].isUpper) /* is class */
                    writefln("\t\t\t%s%s = new C%s(dclass.%s%s);",
                            escapeName(cv.name), "", cv.type, cv.name, "");
                else
                    writefln("\t\t\t%s%s = dclass.%s%s;", escapeName(cv.name), "", cv.name, "");
                writefln("\t\treturn %s%s;", escapeName(cv.name), "");
                writeln("\t}");
            }
            else {
                writeln(");");
            }
        }

        if (genType == GenType.GenClasses) {
            writeln();
            writeln("private: //Variables.");
            writefln("\tconst %s dclass;", cd.name.text);
            foreach (cv; classvars) {
                if (cv.type == "string")
                    cv.type = "const(char)*";

                string typeorname = cv.type;
                if (cv.type in allclasses)
                    typeorname = "I" ~ cv.type;

                writefln("\t%s%s %s;", typeorname, "", escapeName(cv.name));
                // // else
                //     writefln("\t%s%s %s;", declaration.type == "string"
                //             ? "const(char)*" : declaration.type, typePostfix,
                //             escapeName(declaration.name));
            }
        }

        writeln("}");
        writeln();
    }

    override void visit(const StructBody sb) {
        inStruct = true;
        super.visit(sb);
        inStruct = false;
    }

    override void visit(const Declaration d) {
        if (d.attributes.any!(x => x.attribute == tok!"private" || x.attribute == tok!"protected")) {
            return;
        }
        // foreach (ref declaration ; d.declarations) {
        //     if (declaration.mixinDeclaration) {
        //         declaration = null;
        //     }
        // }
        super.visit(d);
    }

    override void visit(const VariableDeclaration vd) {
        inVariable = true;
        isArray = false;
        super.visit(vd);
        inVariable = false;
    }

    override void visit(const Unittest ut) {
        // skip unittests
        return;
    }

    override void visit(const Declarator d) {
        classvars ~= ClassVariable(typename, d.name.text, "", isArray, false, false);
        super.visit(d);
    }

    override void visit(const FunctionDeclaration fd) {
        inFunction = true;
        super.visit(fd);
        inFunction = false;
    }

    override void visit(const MixinTemplateName mtn) {

        auto mixinSymbol = mtn.symbol;
        if (mixinSymbol && getString(mixinSymbol) == "BinaryExpressionBody") {
            MixinTemplateDeclaration mtd = mixins[getString(mixinSymbol)];
            mtd.accept(this);
        }
        super.visit(mtn);

    }

    override void visit(const MixinTemplateDeclaration mtd) {
        // skip mixin templates
        return;
    }

    override void visit(const Type t) {
        isArray = isArray || t.typeSuffixes.any!(x=>x.array);
        super.visit(t);
    }

    override void visit(const Type2 t2) {
        if (inStruct && (inVariable || inFunction)) {
            if (t2.typeIdentifierPart !is null) {
                typename = getString(t2.typeIdentifierPart.identifierOrTemplateInstance.identifier);
                // if (var.type.type2.typeIdentifierPart.identifierOrTemplateInstance.templateInstance !is null) {
                //    // skip variables that are templates
                //    continue;
                // }
                if (typename == "IdType") {
                    typename = "string";
                    return;
                }
                else if (typename == "size_t" || typename == "string")
                    return;
            }
            else {
                typename = str(t2.builtinType);
            }
        }
        super.visit(t2);
    }

    private bool inStruct = false;
    private bool inVariable = false;
    private bool inFunction = false;
    private bool isArray = false;
    private string typename;
}

void old(Module mod, string[] args) {

    // all declarations
    foreach (declaration; mod.declarations) {

        // deal with alias declarations
        if (declaration.aliasDeclaration) {
            if (declaration.aliasDeclaration.initializers) {
                foreach (init; declaration.aliasDeclaration.initializers) {
                    //stderr.writeln("Alias: ",init.name.text);
                    aliasses[init.name.text] = Alias(init.name.text, getString(
                            init.type.type2.typeIdentifierPart.identifierOrTemplateInstance.templateInstance.identifier),
                            init.type.type2.typeIdentifierPart.identifierOrTemplateInstance.templateInstance
                            .templateArguments.templateSingleArgument.token == tok!"true"
                            ? "true" : "false" // 
                            );

                }
            }
            continue;
        }

        // deal with enums
        if (declaration.enumDeclaration) {
            Enum en;
            en.name = declaration.enumDeclaration.name.text;
            foreach (member; declaration.enumDeclaration.enumBody.enumMembers) {
                en.values ~= member.name.text;
            }
            enums ~= en;
        }

        if (!declaration.classDeclaration)
            continue;

        // deal with classes
        Class classType;
        classType.name = declaration.classDeclaration.name.text;

        if (declaration.classDeclaration.templateParameters !is null) {
            // store templated classes for later 
            templatedClasses[classType.name] = declaration;
            continue;
        }
        if (declaration.classDeclaration.baseClassList) {
            foreach (baseClass; declaration.classDeclaration.baseClassList.items) {
                if (getString(
                        baseClass.type2.typeIdentifierPart.identifierOrTemplateInstance.identifier) == "ExpressionNode") {
                    classType.isExpression = true;
                }
            }
        }

        foreach (innerDeclaration; declaration.classDeclaration.structBody.declarations) {
            ClassVariable cv;
            if (innerDeclaration.mixinDeclaration && innerDeclaration.mixinDeclaration.templateMixinExpression
                    && innerDeclaration.mixinDeclaration.templateMixinExpression.mixinTemplateName) {

                auto mixinSymbol = innerDeclaration.mixinDeclaration
                    .templateMixinExpression.mixinTemplateName.symbol;
                if (mixinSymbol && getString(mixinSymbol) == "BinaryExpressionBody") {
                    cv.type = "ExpressionNode";
                    cv.name = "left";
                    classes[classType] ~= cv;
                    cv.name = "right";
                    classes[classType] ~= cv;
                    cv.type = "size_t";
                    cv.name = "line";
                    classes[classType] ~= cv;
                    cv.name = "column";
                    classes[classType] ~= cv;
                }

            }

            // skip any private members
            if (innerDeclaration.attributes.any!(x => x.attribute == tok!"private"
                    || x.attribute == tok!"protected")) {
                continue;
            }

            if (!innerDeclaration.variableDeclaration) {
                continue;
            }
            // variable declarations after here!
            auto var = innerDeclaration.variableDeclaration;

            if (var.type.type2.type !is null) {
                cv.type = getString(
                        var.type.type2.type.type2.typeIdentifierPart
                        .identifierOrTemplateInstance.identifier);
            }
            else if (var.type.type2.typeIdentifierPart !is null) {
                cv.type = getString(
                        var.type.type2.typeIdentifierPart.identifierOrTemplateInstance.identifier);
                if (
                    var.type.type2.typeIdentifierPart
                        .identifierOrTemplateInstance.templateInstance !is null) {
                    // skip variables that are templates
                    continue;
                }
            }
            else {
                cv.type = str(var.type.type2.builtinType);
            }
            // FIXED: JAAPG
            /*	if(auto symbol = var.type.type2.symbol)
				cv.type = getString(var.type.type2.symbol);
			else if(auto chain = var.type.type2.identifierOrTemplateChain)
				cv.type = getString(var.type.type2.symbol); 
			else*/
            //cv.type = str(var.type.type2.builtinType);

            foreach (suffix; var.type.typeSuffixes) {
                if (suffix.array)
                    cv.isArray = true;
            }

            foreach (declarator; var.declarators) {
                cv.name = declarator.name.text;

                classes[classType] ~= cv;
            }
        }
    }

    // TODO verify
    //Add mixed in variables from Declaration.
    {
        Class newClassType;
        newClassType.name = "Declaration";
        ClassVariable cv;
        cv.type = "AliasDeclaration";
        cv.name = "aliasDeclaration";
        classes[newClassType] ~= cv;
        cv.type = "AliasThisDeclaration";
        cv.name = "aliasThisDeclaration";
        classes[newClassType] ~= cv;
        cv.type = "AnonymousEnumDeclaration";
        cv.name = "anonymousEnumDeclaration";
        classes[newClassType] ~= cv;
        cv.type = "AttributeDeclaration";
        cv.name = "attributeDeclaration";
        classes[newClassType] ~= cv;
        cv.type = "ClassDeclaration";
        cv.name = "classDeclaration";
        classes[newClassType] ~= cv;
        cv.type = "ConditionalDeclaration";
        cv.name = "conditionalDeclaration";
        classes[newClassType] ~= cv;
        cv.type = "Constructor";
        cv.name = "constructor";
        classes[newClassType] ~= cv;
        cv.type = "DebugSpecification";
        cv.name = "debugSpecification";
        classes[newClassType] ~= cv;
        cv.type = "Destructor";
        cv.name = "destructor";
        classes[newClassType] ~= cv;
        cv.type = "EnumDeclaration";
        cv.name = "enumDeclaration";
        classes[newClassType] ~= cv;
        cv.type = "EponymousTemplateDeclaration";
        cv.name = "eponymousTemplateDeclaration";
        classes[newClassType] ~= cv;
        cv.type = "FunctionDeclaration";
        cv.name = "functionDeclaration";
        classes[newClassType] ~= cv;
        cv.type = "ImportDeclaration";
        cv.name = "importDeclaration";
        classes[newClassType] ~= cv;
        cv.type = "InterfaceDeclaration";
        cv.name = "interfaceDeclaration";
        classes[newClassType] ~= cv;
        cv.type = "Invariant";
        cv.name = "invariant_";
        classes[newClassType] ~= cv;
        cv.type = "MixinDeclaration";
        cv.name = "mixinDeclaration";
        classes[newClassType] ~= cv;
        cv.type = "MixinTemplateDeclaration";
        cv.name = "mixinTemplateDeclaration";
        classes[newClassType] ~= cv;
        cv.type = "Postblit";
        cv.name = "postblit";
        classes[newClassType] ~= cv;
        cv.type = "PragmaDeclaration";
        cv.name = "pragmaDeclaration";
        classes[newClassType] ~= cv;
        cv.type = "SharedStaticConstructor";
        cv.name = "sharedStaticConstructor";
        classes[newClassType] ~= cv;
        cv.type = "SharedStaticDestructor";
        cv.name = "sharedStaticDestructor";
        classes[newClassType] ~= cv;
        cv.type = "StaticAssertDeclaration";
        cv.name = "staticAssertDeclaration";
        classes[newClassType] ~= cv;
        cv.type = "StaticConstructor";
        cv.name = "staticConstructor";
        classes[newClassType] ~= cv;
        cv.type = "StaticDestructor";
        cv.name = "staticDestructor";
        classes[newClassType] ~= cv;
        cv.type = "StructDeclaration";
        cv.name = "structDeclaration";
        classes[newClassType] ~= cv;
        cv.type = "TemplateDeclaration";
        cv.name = "templateDeclaration";
        classes[newClassType] ~= cv;
        cv.type = "UnionDeclaration";
        cv.name = "unionDeclaration";
        classes[newClassType] ~= cv;
        cv.type = "Unittest";
        cv.name = "unittest_";
        classes[newClassType] ~= cv;
        cv.type = "VariableDeclaration";
        cv.name = "variableDeclaration";
        classes[newClassType] ~= cv;
        cv.type = "VersionSpecification";
        cv.name = "versionSpecification";
        classes[newClassType] ~= cv;
    }

    //Add expressionNode.
    {
        Class newClassType;
        newClassType.name = "ExpressionNode";
        foreach (classType, declarations; classes) {
            if (!classType.isExpression)
                continue;
            ClassVariable cv;
            cv.type = classType.name;
            cv.name = escapeAndLowerName(classType.name);
            classes[newClassType] ~= cv;
        }
    }

    //Add token.
    {
        Class newClassType;
        newClassType.name = "Token";
        ClassVariable cv;

        cv.type = "string";
        cv.name = "text";
        classes[newClassType] ~= cv;
        cv.type = "size_t";
        cv.name = "line";
        classes[newClassType] ~= cv;
        cv.type = "size_t";
        cv.name = "column";
        classes[newClassType] ~= cv;
    }

    // check if variable declarations inside classes are of class type so they can be made an interface
    foreach (ref key, ref val; classes) {
        foreach (ref cv; val) {
            cv.isClass = classes.keys.any!(x => x.name == cv.type);
            cv.isEnum = enums.any!(x => x.name == cv.type);
        }
    }

    // generate header file
    if (args.length > 2 && args[2] == "-h") {
        writeln("#pragma once");
        writeln();

        // writefln("#include <kdemacros.h>");
        // writeln();

        //Forward declarations.
        foreach (classType, declarations; classes)
            writefln("class I%s;", classType.name);
        writeln();

        //Functions.
        writeln("void initDParser();");
        writeln("void deinitDParser();");
        writeln();

        writeln("IModule *parseSourceFile(char *sourceFile, char *sourceData);");
        writeln();

        //Kind enum.
        writeln("enum class Kind");
        writeln("{");
        foreach (classType, declarations; classes)
            writefln("\t%s,", escapeAndLowerName(classType.name));
        writeln("};");
        writeln();

        // other enums
        /* enums are ignored for now
        foreach (en; enums) {
            writeln("enum ",en.name);
            writeln("{");
            foreach(member; en.values) {
                writefln("\t%s,",escapeAndLowerName(member));
            }
            writeln("};");
            writeln();
        }
        */

        //INode.
        writeln("class INode");
        writeln("{");
        writeln("public: //Methods.");
        writeln("	virtual Kind getKind();");
        writeln("	virtual void *getContext();");
        writeln("	virtual void setContext(void *context);");
        writeln("\t");
        writeln("protected: //Methods.");
        writeln("	~INode() {}");
        writeln("};");
        writeln();

        //Interfaces.
        foreach (classType, declarations; classes) {
            writefln("class I%s : public INode", classType.name);
            writeln("{");
            writeln("public: //Methods.");
            foreach (declaration; declarations) {
                if (declaration.type == "") {
                    writefln("\t//Skipping %s.", declaration.name);
                    continue;
                }
                string type = declaration.type == "IdType" ? "string" : (declaration.type[0].isUpper()
                        ? "I" ~ declaration.type : declaration.type);
                if (declaration.isArray) {
                    writefln("\tvirtual size_t num%s%s();", declaration.name[0].toUpper(),
                            declaration.name[1 .. $].replace("_", ""));
                    string name = format("%s%s", declaration.name[0].toUpper(),
                            declaration.name[1 .. $].replace("_", ""));
                    if (name.endsWith("ses"))
                        name = name[0 .. $ - 3];
                    else if (name.endsWith("xes"))
                        name = name[0 .. $ - 2];
                    else if (name.endsWith("s"))
                        name = name[0 .. $ - 1];
                    writefln("\tvirtual %s %sget%s(size_t index);", type == "string" ? "const char" : type,
                            declaration.type[0].isUpper() || type == "string" ? "*" : "", name);
                }
                else
                    writefln("\tvirtual %s %sget%s%s(%s);", type == "string" ? "const char" : type,
                            declaration.type[0].isUpper() || type == "string" ? "*" : "",
                            declaration.name[0].toUpper(),
                            declaration.name[1 .. $].replace("_", ""), declaration.args);
            }
            writeln("\t");
            writeln("protected: //Methods.");
            writefln("	~I%s() {}", classType.name);
            writeln("};");
            writeln();
        }
        return;
    }

    // generate D Wrapper
    writeln("module astWrapper;");
    writeln();
    writeln("import std.string;");
    writeln();
    writeln("import dparse.lexer;");
    writeln("import dparse.parser;");
    writeln("import dparse.ast;");
    writeln();

    // writeln("import std.variant : Algebraic;");
    // writeln(); 

    writeln("template ContextMethods()");
    writeln("{");
    writeln("	extern(C++) void* getContext()");
    writeln("	{");
    writeln("		return context;");
    writeln("	}");
    writeln("	extern(C++) void setContext(void* context)");
    writeln("	{");
    writeln("		this.context = context;");
    writeln("	}");
    writeln("	__gshared void* context;");
    writeln("}");
    writeln();

    //Kind enum.
    writeln("enum Kind");
    writeln("{");
    foreach (classType, declarations; classes)
        writefln("\t%s,", escapeAndLowerName(classType.name));
    writeln("}");
    writeln();

    // other enums
    /* enums are ignored for now
    foreach (en; enums) {
        writeln("enum ",en.name);
        writeln("{");
        foreach(member; en.values) {
            writefln("\t%s,",escapeAndLowerName(member));
        }
        writeln("}");
        writeln();
    }
    */
    //INode.
    writeln("extern(C++) interface INode");
    writeln("{");
    writeln("	Kind getKind();");
    writeln("	void* getContext();");
    writeln("	void setContext(void* context);");
    writeln("}");
    writeln();

    //Interfaces.
    foreach (classType, declarations; classes) {
        writefln("extern(C++) interface I%s : INode", classType.name);
        writeln("{");
        writeInterfaceDecls(classType, declarations);
        writeln("}");
        writeln();
    }

    // foreach(declaration; templatedClasses) {
    //     Class c;
    //     c.name = declaration.init.text;
    //     c.isExpression = false;
    //     classes ~= ;
    // }

    // alias template interface classes
    foreach (init; aliasses) {
        // alias points to template classes to generate
        writefln("extern(C++) interface I%s : INode", init.name);
        writeln("{");
        // if (init.templateargs == "true") {
        //     writeInterfaceDecls(classType, declarations);
        // } else {
        //     writeInterfaceDecls(classType, declarations);
        // }
        writeln("}");
        writeln();
    }

    //Classes.
    foreach (classType, declarations; classes) {

        writefln("class C%s : I%s", classType.name, classType.name);
        writeln("{");
        writeln("public: //Methods.");
        writefln("\tthis(const %s dclass)", classType.name);
        writeln("\t{");
        writeln("\t\tthis.dclass = dclass;");
        writeln("\t}");
        writeln("\t");
        writeln("\textern(C++) Kind getKind()");
        writeln("\t{");
        writefln("\t\treturn Kind.%s;", escapeAndLowerName(classType.name));
        writeln("\t}");
        writeln("\t");
        writeln("\tmixin ContextMethods;");
        foreach (declaration; declarations) {
            if (declaration.type == "") {
                writefln("\t//Skipping %s.", declaration.name);
                continue;
            }
            writeln("\t");

            string type;
            if (declaration.type == "IdType") {
                type = "string";
            }
            //            else if (declaration.type[0].isUpper()) {
            else if (declaration.isClass) {
                type = "I" ~ declaration.type;
            }
            else
                type = declaration.type;

            // for now just ignore any enums
            if (!declaration.isEnum) {
                if (declaration.isArray) {
                    writefln("\textern(C++) size_t num%s%s()", declaration.name[0].toUpper(),
                            declaration.name[1 .. $].replace("_", ""));
                    writeln("\t{");
                    writefln("\t\tif(!dclass.%s)", escapeName(declaration.name));
                    writeln("\t\t\treturn 0;");
                    writefln("\t\treturn dclass.%s.length;", declaration.name);
                    writeln("\t}");
                    string name = format("%s%s", declaration.name[0].toUpper(),
                            declaration.name[1 .. $].replace("_", ""));
                    if (name.endsWith("ses"))
                        name = name[0 .. $ - 3];
                    else if (name.endsWith("xes"))
                        name = name[0 .. $ - 2];
                    else if (name.endsWith("s"))
                        name = name[0 .. $ - 1];
                    writefln("\textern(C++) %s get%s(size_t index)",
                            type == "string" ? "const(char)*" : type, name);
                }
                else
                    writefln("\textern(C++) %s get%s%s(%s)", type == "string" ? "const(char)*" : type,
                            declaration.name[0].toUpper(),
                            declaration.name[1 .. $].replace("_", ""), declaration.args);
                writefln("\t{");
                string typePostfix = declaration.isArray ? "[index]" : "";
                if (classType.name == "ExpressionNode") {
                    writefln("\t\tif(!%s && cast(%s)dclass)",
                            escapeName(declaration.name), declaration.type);
                    writefln("\t\t\t%s = new C%s(cast(%s)dclass);",
                            escapeName(declaration.name), declaration.type, declaration.type);
                }
                else {
                    if (declaration.isArray)
                        writefln("\t\tif(index !in %s)", escapeName(declaration.name));
                    else
                        writefln("\t\tif(!%s%s)", escapeName(declaration.name), declaration.type == "Token"
                                || declaration.type == "string"
                                ? "" : format(" && dclass.%s", declaration.name));
                    if (declaration.type == "IdType")
                        writefln("\t\t\t%s%s = toStringz(str(dclass.%s%s));",
                                escapeName(declaration.name), typePostfix,
                                declaration.name, typePostfix);
                    else if (declaration.type[0].isUpper())
                        writefln("\t\t\t%s%s = new C%s(dclass.%s%s);", escapeName(declaration.name),
                                typePostfix, declaration.type, declaration.name, typePostfix);
                    else if (type == "string")
                        writefln("\t\t\t%s%s = toStringz(dclass.%s%s);",
                                escapeName(declaration.name), typePostfix,
                                declaration.name, typePostfix);
                    else
                        writefln("\t\t\t%s%s = dclass.%s%s;", escapeName(declaration.name),
                                typePostfix, declaration.name, typePostfix);
                }
                writefln("\t\treturn %s%s;", escapeName(declaration.name), typePostfix);
                writefln("\t}");
            }
        }
        writeln();
        writeln("private: //Variables.");
        writefln("\tconst %s dclass;", classType.name);
        foreach (declaration; declarations) {
            if (declaration.type == "") {
                writefln("\t//Skipping %s.", declaration.name);
                continue;
            }
            string typePostfix = declaration.isArray ? "[size_t]" : "";
            if (declaration.type == "IdType")
                writefln("\tconst(char)*%s %s;", typePostfix, escapeName(declaration.name));
            else if (declaration.isClass)
                writefln("\tI%s%s %s;", declaration.type, typePostfix,
                        escapeName(declaration.name));
            else
                writefln("\t%s%s %s;", declaration.type == "string"
                        ? "const(char)*" : declaration.type, typePostfix,
                        escapeName(declaration.name));
        }
        writeln("}");
        writeln();
    }

    // alias template implementation classes
    foreach (init; aliasses) {
        // alias points to template classes to generate
        writefln("class C%s : I%s", init.name, init.templatename);
        writeln("{");
        writeln("}");
        writeln();
    }
}

void writeInterfaceDecls(Class classType, ClassVariable[] declarations) {
    foreach (declaration; declarations) {

        if (declaration.type == "") {
            writefln("\t//Skipping %s.", declaration.name);
            continue;
        }
        string type;
        if (declaration.type == "IdType") {
            type = "string";
        }
        //            else if (declaration.type[0].isUpper()) {
        else if (declaration.isClass) {
            type = "I" ~ declaration.type;
        }
        else
            type = declaration.type;

        // for now ignore any enums
        if (!declaration.isEnum) {
            if (declaration.isArray) {
                writefln("\tsize_t num%s%s();", declaration.name[0].toUpper(),
                        declaration.name[1 .. $].replace("_", ""));
                string name = format("%s%s", declaration.name[0].toUpper(),
                        declaration.name[1 .. $].replace("_", ""));
                if (name.endsWith("ses"))
                    name = name[0 .. $ - 3];
                else if (name.endsWith("xes"))
                    name = name[0 .. $ - 2];
                else if (name.endsWith("s"))
                    name = name[0 .. $ - 1];
                writefln("\t%s get%s(size_t index);", type == "string" ? "const(char)*" : type,
                        name);
            }
            else
                writefln("\t%s get%s%s(%s);", type == "string" ? "const(char)*" : type,
                        declaration.name[0].toUpper(),
                        declaration.name[1 .. $].replace("_", ""), declaration.args);
        }
    }
}
