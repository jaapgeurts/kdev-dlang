module cppwrappergenerator;

import std.stdio;
import std.uni;
import std.string;
import std.algorithm.searching;
import std.algorithm.mutation;

import std.typecons;

import dparse.lexer;
import dparse.parser;
import dparse.ast;
import dparse.rollback_allocator;

private enum keywords = [
        "abstract", "alias", "align", "asm", "assert", "auto", "bool", "break",
        "byte", "case", "cast", "catch", "cdouble", "cent", "cfloat", "char",
        "class", "const", "continue", "creal", "dchar", "debug", "default",
        "delegate", "delete", "deprecated", "do", "double", "else", "enum",
        "export", "extern", "false", "final", "finally", "float", "for", "foreach",
        "foreach_reverse", "function", "goto", "idouble", "if", "ifloat",
        "immutable", "import", "in", "inout", "int", "interface", "invariant",
        "ireal", "is", "lazy", "long", "macro", "mixin", "module", "new",
        "nothrow", "null", "out", "override", "package", "pragma", "private",
        "protected", "public", "pure", "real", "ref", "return", "scope", "shared",
        "short", "static", "struct", "super", "switch", "synchronized", "template",
        "this", "throw", "true", "try", "typedef", "typeid", "typeof", "ubyte",
        "ucent", "uint", "ulong", "union", "unittest", "ushort", "version", "void",
        "wchar", "while", "with", "__DATE__", "__EOF__", "__FILE__",
        "__FILE_FULL_PATH__", "__FUNCTION__", "__gshared", "__LINE__",
        "__MODULE__", "__parameters", "__PRETTY_FUNCTION__", "__TIME__",
        "__TIMESTAMP__", "__traits", "__vector", "__VENDOR__", "__VERSION__"
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

ClassDeclaration[string] templatedClasses;

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

Enum[string] enumTable;

ClassVariable[] classvars;
string[string] allclasses;
string[] allexpressionclasses;

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

private enum OutputFormat {
    DModule,
    CHeader
}

private OutputFormat outFormat = OutputFormat.DModule;

// rewrite using visitor pattern
void main(string[] args) {

    LexerConfig config;
    config.fileName = args[1];

    if (args.length == 3 && args[2] == "-h")
        outFormat = OutputFormat.CHeader;

    RollbackAllocator allocator;

    auto file = File(args[1], "r");
    auto source = new ubyte[](file.size());
    file.rawRead(source);
    auto tokens = getTokensForParser(source, config,
            new StringCache(StringCache.defaultBucketCount));

    auto mod = parseModule(tokens, config.fileName, &allocator);

    //    old(mod,args);
    //    return;

    // add Token class
    Declaration d = new Declaration();
    // Text accessor
    d.classDeclaration = makeClassDeclaration("Token",
        makeVariableDecl("string", "text"), // the token text
        makeVariableDecl("IdType","type"),
        makeVariableDecl("size_t", "line"),
        makeVariableDecl("size_t", "column"));
    mod.declarations ~= d;

    // First pass
    mod.accept(new pass1Visitor());

    // Add ExpressionNode
    d = new Declaration();
    ClassDeclaration cd = makeClassDeclaration("ExpressionNode");
    foreach (expr; allexpressionclasses) {
        cd.structBody.declarations ~= makeVariableDecl(expr, escapeAndLowerName(expr));
    }
    d.classDeclaration = cd;
    mod.declarations ~= d;

    // Output header
    writeHeader();

    writeExtras();

    // Second pass
    WrapperGenVisitor visitor = new WrapperGenVisitor(WrapperGenVisitor.GenType.GenInterfaces);
    mod.accept(visitor);

    writeInterfaceForTemplate(visitor);

    if (outFormat == OutputFormat.DModule) {
        WrapperGenVisitor visitor2 = new WrapperGenVisitor(WrapperGenVisitor.GenType.GenClasses);

        mod.accept(visitor2); // TODO: reset and re-use visitor instead of creating new one
        writeClassForTemplate(visitor2);
    }

}

void writeHeader() {

    if (outFormat == OutputFormat.DModule) {
        // generate D Wrapper
        writeln("module astWrapper;");
        writeln();
        writeln("import std.string;");
        writeln();
        writeln("import dparse.lexer;");
        writeln("import dparse.parser;");
        writeln("import dparse.ast;");
        writeln();
        writeln("import std.stdio;");
//        writeln("import core.stdc.string;");
        writeln();
    }
    else {
        writeln("#pragma once");
        writeln();
    }

}

void writeExtras() {

    if (outFormat == OutputFormat.DModule) {

        writeln("template ContextMethods()");
        writeln("{");
        writeln("	extern(C++) void* getContext()");
        writeln("	{");
        version(log) writeln("\t\twritefln(\"getContext(): %x\",context);");
        writeln("		return context;");
        writeln("	}");
        writeln("	extern(C++) void setContext(void* context)");
        writeln("	{");
        version(log) writeln("\t\twritefln(\"setContext(): %x\",context);");
        writeln("		this.context = context;");
        writeln("	}");

        writeln("	void* context;");
        writeln("}");
        writeln();

        writeln("enum ParseMsgType { Warning, Error }");
        writeln();

         // IParseMessage
        writeln("extern(C++) interface IParseMessage");
        writeln("{");
        writeln("\tconst(char)* getMessage();");
        writeln("\tParseMsgType getType();");
        writeln("\tsize_t getLine();");
        writeln("\tsize_t getColumn();");
        writeln("}");
        writeln();

        // IParseResult
        writeln("extern(C++) interface IParseResult");
        writeln("{");
        writeln("\tIModule ast();");
        writeln("\tbool succes();");
        writeln("\tIParseMessage message(size_t index);");
        writeln("\tsize_t messageCount();");
        writeln("\tvoid release();");
        writeln("}");
        writeln();

        // Enum kind
        writeln("enum Kind");
        writeln("{");
        foreach (cv; allclasses) {
            writefln("\t%s,", escapeAndLowerName(cv));
        }
        foreach (aliasclass; aliasses) {
            writefln("\t%s,", escapeAndLowerName(aliasclass.name));
        }
        writeln("}");
        writeln();

        // Enum kind
        writeln("string[] kindEnumName = ");
        writeln("[");
        foreach (cv; allclasses) {
            writefln("\t\"%s\",", escapeAndLowerName(cv));
        }
        foreach (aliasclass; aliasses) {
            writefln("\t\"%s\",", escapeAndLowerName(aliasclass.name));
        }
        writeln("];");
        writeln();
        writeln("extern(C++) const(char)* kindToString(Kind kind)");
        writeln("{");
        writeln("\treturn kindEnumName[kind].toStringz;");
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
    else { // output format is C header

        //Forward declarations.
        foreach (classname; allclasses)
            writefln("class I%s;", classname);

        foreach (aliasclass; aliasses)
            writefln("class I%s;", aliasclass.name);
        writeln();

        //Functions.
        writeln("void initDParser();");
        writeln("void deinitDParser();");
        writeln();

        writeln("enum class ParseMsgType { Warning, Error };");
        writeln();

    // IParseMessage
        writeln("class IParseMessage");
        writeln("{");
        writeln("public:");

        writeln("\tvirtual const char* getMessage();");
        writeln("\tvirtual ParseMsgType getType();");
        writeln("\tvirtual size_t getLine();");
        writeln("\tvirtual size_t getColumn();");
        writeln("protected: //Methods.");
        writeln("\t~IParseMessage() = default;");
        writeln("};");
        writeln();

        // IParseResult
        writeln("class IParseResult");
        writeln("{");
        writeln("public:");
        writeln("\tvirtual IModule* ast() = 0;");
        writeln("\tvirtual bool succes() = 0;");
        writeln("\tvirtual IParseMessage* message(uint index) = 0;");
        writeln("\tvirtual uint messageCount() = 0;");
        writeln("\tvirtual void release() = 0;");
        writeln("protected: //Methods.");
        writeln("\t~IParseResult() = default;");
        writeln("};");
        writeln();

        writeln("extern \"C\" {");
        writeln("\tIParseResult *parseSourceFile(const char *sourceFile, const char *sourceData);");
//        writeln("\tvoid freeAst(const char* sourceFile);");
        writeln("}");
        writeln();

        //Kind enum.
        writeln("enum class Kind");
        writeln("{");
        int i=0;
        foreach (classname; allclasses) {
            if (classname == "Register")
                classname ~= "_";
            writefln("\t%s, // %d", escapeAndLowerName(classname),i);
            i++;
        }
        foreach (aliasclass; aliasses) {
            writefln("\t%s, // %d", escapeAndLowerName(aliasclass.name),i);
            i++;
        }
        writeln("};");
        writeln();

        writeln("const char* kindToString(Kind kind);");
        writeln();


        // all collected enums
        foreach (en; enumTable) {
            writeln("enum class ", en.name, " : int8_t {");
            foreach (val; en.values)
                writeln(val, ",");
            writeln("};");
            writeln();
        }

        //INode.
        writeln("class INode");
        writeln("{");
        writeln("public: //Methods.");
        writeln("	virtual Kind getKind();");
        writeln("	virtual void *getContext();");
        writeln("	virtual void setContext(void *context);");
        writeln("\t");
        writeln("protected: //Methods.");
        writeln("	~INode() = default;");
        writeln("};");
        writeln();
    }

}

/** this function produces interfaces for two foreach aliases which expand to template class ForEach */
void writeInterfaceForTemplate(WrapperGenVisitor visitor) {
    foreach (aliasclass; aliasses) {

        ClassDeclaration cd = templatedClasses[aliasclass.templatename];

        if (outFormat == OutputFormat.DModule)
            writefln("extern(C++) interface I%s : INode", aliasclass.name);
        else
            writefln("class I%s : public INode", aliasclass.name);
        writeln("{");

        classvars.length = 0;
        cd.accept(visitor);

        if (aliasclass.templateargs == "true") {
            // TODO: quick fix. instead of evaluating the static if in the template
            classvars = classvars.remove(
                    classvars.countUntil!(c => c.name == "declarationOrStatement"));
            // remove the variables instead
        }
        else {
            classvars = classvars.remove(classvars.countUntil!(c => c.name == "declarations"));
            classvars = classvars.remove(classvars.countUntil!(c => c.name == "style"));
        }

        writeln("public:");
        foreach (cv; classvars) {

            visitor.writeInterfaceMethods(cv);
        }

        if (outFormat == OutputFormat.DModule) {
            writeln("\tsize_t getStartLine();");
            writeln("\tsize_t getStartColumn();");
            writeln("\tsize_t getEndLine();");
            writeln("\tsize_t getEndColumn();");
            writeln("}");
        }
        else {
            writeln("\tvirtual size_t getStartLine();");
            writeln("\tvirtual size_t getStartColumn();");
            writeln("\tvirtual size_t getEndLine();");
            writeln("\tvirtual size_t getEndColumn();");
            writeln();
            writeln("protected: //Methods.");
            writefln("\tvirtual ~I%s() = default;", aliasclass.name);
            writeln("};");
        }

        writeln();

    }
}

/** this function produces classes for two foreach aliases which expand to template class ForEach */
void writeClassForTemplate(WrapperGenVisitor visitor) {
    foreach (aliasclass; aliasses) {
        ClassDeclaration cd = templatedClasses[aliasclass.templatename];

        writefln("class C%s : I%s", aliasclass.name, aliasclass.name);
        writeln("{");

        visitor.writeClassConstructor(cd, aliasclass.nullable);

        writeln("\textern(C++) size_t getStartLine() { return dclass.tokens[0].line; }");
        writeln("\textern(C++) size_t getStartColumn() { return dclass.tokens[0].column; }");
        writeln("\textern(C++) size_t getEndLine() { return dclass.tokens[$-1].line; }");
        writeln("\textern(C++) size_t getEndColumn() { return dclass.tokens[$-1].column; }");

        classvars.length = 0;
        cd.accept(visitor);

        if (aliasclass.templateargs == "true") {
            // TODO: quick fix. instead of evaluating the static if in the template
            classvars = classvars.remove(
                    classvars.countUntil!(c => c.name == "declarationOrStatement"));
            // remove the variables instead
        }
        else {
            classvars = classvars.remove(classvars.countUntil!(c => c.name == "declarations"));
            classvars = classvars.remove(classvars.countUntil!(c => c.name == "style"));
        }

        foreach (cv; classvars) {

            visitor.writeClassMethods(cv, false);
        }

        visitor.writeClassVariables(cd, aliasclass.nullable);

        writeln("}");
        writeln();
    }
}

ClassDeclaration makeClassDeclaration(string className, Declaration[] decls...) {
    ClassDeclaration cd = new ClassDeclaration();
    cd.name = Token(tok!"identifier", className, 0, 0, 0);
    cd.structBody = new StructBody();
    foreach (decl; decls) {
        cd.structBody.declarations ~= decl;
    }
    return cd;
}

Declaration makeVariableDecl(string tname, string vname) {
    Declaration d = new Declaration();
    VariableDeclaration vd = new VariableDeclaration();
    Type t = new Type();
    t.type2 = new Type2();
    t.type2.typeIdentifierPart = new TypeIdentifierPart();
    t.type2.typeIdentifierPart.identifierOrTemplateInstance = new IdentifierOrTemplateInstance();
    t.type2.typeIdentifierPart.identifierOrTemplateInstance.identifier = Token(
            tok!"identifier", tname, 0, 0, 0);
    vd.type = t;
    Declarator dc = new Declarator();
    dc.name = Token(tok!"identifier", vname, 0, 0, 0);
    vd.declarators ~= dc;
    d.variableDeclaration = vd;
    return d;
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


        if (cd.templateParameters !is null) {
            // store templated classes for different use
            templatedClasses[cd.name.text] = cast(ClassDeclaration) cd;
        }  else {
            allclasses[cd.name.text] = cd.name.text;
        }

        if (cd.baseClassList) {
            foreach (baseClass; cd.baseClassList.items) {
                if (getString(
                        baseClass.type2.typeIdentifierPart.identifierOrTemplateInstance.identifier) == "ExpressionNode") {
                    allexpressionclasses ~= cd.name.text;
                }
            }
        }
    }

    override void visit(const AliasDeclaration ad) {

        const AliasInitializer init = ad.initializers[0];
        string name = init.name.text;
        // NOTE: this only works for the specific Foreach template class
        // where the template param is either true of false
        aliasses[name] = Alias(name,
                getString(init.type.type2.typeIdentifierPart.identifierOrTemplateInstance.templateInstance.identifier),
                init.type.type2.typeIdentifierPart.identifierOrTemplateInstance.templateInstance
                .templateArguments.templateSingleArgument.token == tok!"true" ? "true" : "false");
    }

    override void visit(const EnumDeclaration ed) {
        Enum en = Enum(ed.name.text);
        foreach (val; ed.enumBody.enumMembers) {
            en.values ~= val.name.text;
        }
        enumTable[ed.name.text] = en;
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

        //        stderr.writeln("=>"  , templateArgs,"<=");

        if (cd.name.text in templatedClasses) {
            // writeln("/* ",cd.name.text, " moved to bottom is a template class. See at bottom */");
            // writeln();
            return;
        }

        //stderr.writeln("=>"  , templateArgs,"<=");

        if (genType == GenType.GenInterfaces) {
            if (outFormat == OutputFormat.DModule) {
                writefln("extern(C++) interface I%s : INode", cd.name.text);
                writeln("{");
                if (cd.name.text != "Token") {
                    writeln("\tsize_t getStartLine();");
                    writeln("\tsize_t getStartColumn();");
                    writeln("\tsize_t getEndLine();");
                    writeln("\tsize_t getEndColumn();");
                }
            }
            else {
                writefln("class I%s : public INode", cd.name.text);
                writeln("{");
                writeln("public:");
                if (cd.name.text != "Token") {
                    writeln("\tvirtual size_t getStartLine();");
                    writeln("\tvirtual size_t getStartColumn();");
                    writeln("\tvirtual size_t getEndLine();");
                    writeln("\tvirtual size_t getEndColumn();");
                }
            }

        }
        else {
            writefln("class C%s : I%s", cd.name.text, cd.name.text);
            writeln("{");

            writeClassConstructor(cd, Nullable!Alias.init);

            if (cd.name.text != "Token") {
                writeln("\textern(C++) size_t getStartLine() { return dclass.tokens[0].line; }");
                writeln("\textern(C++) size_t getStartColumn() { return dclass.tokens[0].column; }");
                writeln("\textern(C++) size_t getEndLine() { return dclass.tokens[$-1].line; }");
                writeln("\textern(C++) size_t getEndColumn() { return dclass.tokens[$-1].column; }");
            }
            writeln();
        }

        if (cd.name.text == "Declaration") {
            writeDeclarationMembers();
        }

        classvars.length = 0;
        super.visit(cd);

        foreach (cv; classvars) {
            if (genType == GenType.GenInterfaces)
                writeInterfaceMethods(cv);
            else
                writeClassMethods(cv, cd.name.text == "ExpressionNode");
        }

        if (genType == GenType.GenClasses) {
            writeClassVariables(cd, Nullable!Alias.init);
        }

        if (outFormat == OutputFormat.CHeader) {
            writeln("protected: //Methods.");
            writefln("\tvirtual ~I%s() {}", cd.name.text);
        }

        // write class closing
        if (outFormat == OutputFormat.DModule)
            writeln("}");
        else
            writeln("};");
        writeln();
    }

    override void visit(const StructBody sb) {
        inStruct = true;
        super.visit(sb);
        inStruct = false;
    }

    override void visit(const Declaration d) {
        if (d.attributes.any!(x => x.attribute == tok!"private"
                || x.attribute == tok!"protected" || x.attribute == tok!"abstract")) {
            return;
        }

        // don't care what's inside functions
        if (inFunction)
            return;
        super.visit(d);
    }

    override void visit(const VariableDeclaration vd) {
        inVariable = true;
        isArray = false;
        super.visit(vd);
        inVariable = false;
    }

    override void visit(const Declarator d) {
        // TODO: move the variables. Fill ClassVariable as we visit
        classvars ~= ClassVariable(typename, d.name.text, "", isArray, false, false);
        super.visit(d);
    }

    override void visit(const FunctionDeclaration fd) {
        inFunction = true;
        super.visit(fd);
        inFunction = false;
    }

    // override void visit(const StaticIfCondition sic) {
    //     stderr.writeln("yes");
    // }

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
        isArray = isArray || t.typeSuffixes.any!(x => x.array);
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
                if (typename == "size_t" || typename == "string")
                    return;
            }
            else {
                typename = str(t2.builtinType);
            }
        }
        super.visit(t2);
    }

    override void visit(const Unittest ut) {
        // skip unittests
        return;
    }

    void writeClassConstructor(const ClassDeclaration cd, Nullable!Alias aliasclass) {

        writeln("public: //Methods.");
        string classname = cd.name.text;
        if (!aliasclass.isNull)
            classname = aliasclass.get.templatename ~ "!" ~ aliasclass.get.templateargs;
        writefln("\tthis(const %s dclass)", classname);
        writeln("\t{");
        writeln("\t\tthis.dclass = dclass;");
        writeln("\t}");
        writeln("\t");
        writeln("\textern(C++) Kind getKind()");
        writeln("\t{");
        if (!aliasclass.isNull)
            writefln("\t\treturn Kind.%s;", escapeAndLowerName(aliasclass.get.name));
        else
            writefln("\t\treturn Kind.%s;", escapeAndLowerName(classname));
        writeln("\t}");
        writeln("\t");
        writeln("\tmixin ContextMethods;");
        writeln();

    }

    void writeInterfaceMethods(ClassVariable cv) {

        string[] signatures = getSignatures(cv);

        foreach (signature; signatures) {
            writeln("\t", outFormat == OutputFormat.CHeader ? "virtual " : "", signature, ";");
        }

    }

    void writeClassMethods(ClassVariable cv, bool isExpressionNode) {

        string[] signatures = getSignatures(cv);
        string typePostfix = cv.isArray ? "[index]" : "";

        if (cv.isArray) {
            writeln("\textern(C++) ", signatures[0]);
            writeln("\t{");
            version(log) writefln("\t\twriteln(\"=>%s\");", signatures[0]);
            writefln("\t\tif (!dclass.%s)", escapeName(cv.name));
            writeln("\t\t\treturn 0;");
            writefln("\t\treturn dclass.%s.length;", escapeName(cv.name));
            writeln("\t}");
            writeln();

            writeln("\textern(C++) ", signatures[1]);
            writeln("\t{");
            version(log) writefln("\t\twriteln(\"=>%s\");", signatures[0]);
            writefln("\t\tif (index !in %s)", escapeName(cv.name));
        }
        else {

            writeln("\textern(C++) ", signatures[0]);
            writeln("\t{");
            version(log) writefln("\t\twriteln(\"=>%s\");", signatures[0]);

            if (isExpressionNode)
                writefln("\t\tif (!%s && cast(%s)dclass)", escapeName(cv.name), cv.type);
            else if (cv.type == "string")
                writefln("\t\tif (!%s && dclass.%s)", escapeName(cv.name), escapeName(cv.name));
            else
                writefln("\t\tif (!%s%s)", escapeName(cv.name), cv.type == "Token"
                         || cv.type == "string" ? "" : format(" && dclass.%s", escapeName(cv.name)));
        }

        if (isExpressionNode)
            writefln("\t\t\t%s = new C%s(cast(%s)dclass);", escapeName(cv.name), cv.type, cv.type);
        else if (cv.type == "IdType")
            writefln("\t\t\t%s%s = toStringz(str(dclass.%s%s));",
                    escapeName(cv.name), typePostfix, escapeName(cv.name), typePostfix);
        else if (cv.type == "string")
            writefln("\t\t\t%s%s = toStringz(dclass.%s%s);", escapeName(cv.name),
                    typePostfix, escapeName(cv.name), typePostfix,escapeName(cv.name), typePostfix);
        else if (cv.type in enumTable)
            writefln("\t\t\t%s%s = dclass.%s%s;", escapeName(cv.name),
                    typePostfix, escapeName(cv.name), typePostfix);
        else if (cv.type[0].isUpper) /* is class */
            writefln("\t\t\t%s%s = new C%s(dclass.%s%s);", escapeName(cv.name),
                    typePostfix, cv.type, escapeName(cv.name), typePostfix);
        else
            writefln("\t\t\t%s%s = dclass.%s%s;", escapeName(cv.name),
                    typePostfix, escapeName(cv.name), typePostfix);

        writefln("\t\treturn %s%s;", escapeName(cv.name), typePostfix);
        writeln("\t}");
        writeln();

    }

    string[] getSignatures(ClassVariable cv) {

        string[] result;

        if (cv.type == "string" || cv.type == "IdType") {
            if (outFormat == OutputFormat.DModule)
                cv.type = "const(char)*";
            else
                cv.type = "const char*";
        }

        string typeorname = cv.type;
        if (cv.type in allclasses || cv.type in aliasses) {
            typeorname = "I" ~ cv.type;
            if (outFormat == OutputFormat.CHeader)
                typeorname ~= "*";
        }

        if (cv.isArray) {
            // signature 0
            result ~= format("size_t num%s%s()", cv.name[0].toUpper(),
                    cv.name[1 .. $].replace("_", ""));
            // signature 1
            string name = format("%s%s", cv.name[0].toUpper(), cv.name[1 .. $].replace("_", ""));
            if (name.endsWith("ses"))
                name = name[0 .. $ - 3];
            else if (name.endsWith("xes"))
                name = name[0 .. $ - 2];
            else if (name.endsWith("s"))
                name = name[0 .. $ - 1];
            result ~= format("%s get%s(size_t index)", typeorname, name);
        }
        else {
            // signature 0
            result ~= format("%s get%s%s(%s)", typeorname, cv.name[0].toUpper,
                    cv.name[1 .. $].replace("_", ""), cv.args);
        }
        return result;
    }

    void writeDeclarationMembers() {

        foreach (member; declarationMembers) {
            if (genType == genType.GenInterfaces)
                writeInterfaceMethods(ClassVariable(member,
                        cast(char)(member[0].toLower) ~ member[1 .. $], "", false, true, false));
            else
                writeClassMethods(ClassVariable(member,
                        cast(char)(member[0].toLower) ~ member[1 .. $], "", false, true, false),
                        false);
        }
    }

    void writeClassVariables(const ClassDeclaration cd, Nullable!Alias aliasclass) {
        writeln();
        writeln("private: // Variables.");
        string classname = cd.name.text;
        if (!aliasclass.isNull) {
            if (classname == aliasclass.get.templatename)
                classname = aliasclass.get.templatename ~ "!" ~ aliasclass.get.templateargs;
        }
        writefln("\tconst %s dclass;", classname);
        foreach (cv; classvars) {

            string typePostfix = cv.isArray ? "[size_t]" : "";

            if (cv.type == "string" || cv.type == "IdType")
                cv.type = "const(char)*";

            string typeorname = cv.type;
            if (cv.type in allclasses || cv.type == "Token" || cv.type in aliasses)
                typeorname = "I" ~ cv.type;

            writefln("\t%s%s %s;", typeorname, typePostfix, escapeName(cv.name));

        }
        if (cd.name.text == "Declaration") {
            foreach (member; declarationMembers) {
                writefln("\tI%s %s;", member,
                        escapeName(cast(char)(member[0].toLower) ~ member[1 .. $]));
            }
        }
    }

    private bool inStruct = false;
    private bool inVariable = false;
    private bool inFunction = false;
    private bool isArray = false;
    private string typename;

    private string[] declarationMembers = [
        "AliasDeclaration", "AliasAssign", "AliasThisDeclaration",
        "AnonymousEnumDeclaration", "AttributeDeclaration",
        "ClassDeclaration", "ConditionalDeclaration", "Constructor",
        "DebugSpecification", "Destructor", "EnumDeclaration",
        "EponymousTemplateDeclaration", "FunctionDeclaration", "ImportDeclaration",
        "InterfaceDeclaration", "Invariant", "MixinDeclaration",
        "MixinTemplateDeclaration", "Postblit", "PragmaDeclaration",
        "SharedStaticConstructor", "SharedStaticDestructor",
        "StaticAssertDeclaration", "StaticConstructor", "StaticDestructor",
        "StructDeclaration", "TemplateDeclaration", "UnionDeclaration", "Unittest",
        "VariableDeclaration", "VersionSpecification", "StaticForeachDeclaration"
    ];
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
            enumTable[en.name] = en;
        }

        if (!declaration.classDeclaration)
            continue;

        // deal with classes
        Class classType;
        classType.name = declaration.classDeclaration.name.text;

        if (declaration.classDeclaration.templateParameters !is null) {
            // store templated classes for later
            ClassDeclaration decl = cast(ClassDeclaration) declaration.classDeclaration();
            templatedClasses[classType.name] = decl;
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
            cv.isEnum = (cv.type in enumTable) !is null;
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

    // ************************************
    // generate D Wrapper
    // ************************************
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
    writeln("	void* context;");
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

        // TODO: replace with WriteClassConstructor?
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
            if (declaration.isClass) {
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
        if (declaration.isClass) {
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
