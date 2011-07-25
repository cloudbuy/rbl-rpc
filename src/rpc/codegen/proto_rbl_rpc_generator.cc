#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/compiler/cpp/cpp_generator.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/printer.h>

#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include <boost/tokenizer.hpp>

#include <iostream>

using namespace google::protobuf::compiler::cpp;
using namespace google::protobuf::compiler;
using namespace google::protobuf;
using namespace google::protobuf::io;

using namespace boost::filesystem;

namespace {
  void skel_function(Printer & gen_out, const MethodDescriptor * md)
  {
    gen_out.Print("bool $METHOD_NAME$(t_client_cookie **,ClientData *,$I$,$O$){}\n",
      "METHOD_NAME",md->name(),
      "I",md->input_type()->name() ,
      "O",md->output_type()->name() ); 
  }

  void create_dispatch_functions(Printer & gen_out, const ServiceDescriptor * sd)
  {
    
    for(int i=0; i < sd->method_count(); ++i)
    {
      const MethodDescriptor * method = sd->method(i);
      gen_out.Print("template<typename T_IMPL>\n");
      gen_out.Print("bool $SERV_NAME$_$METHOD_NAME$(T_IMPL & impl,ClientCookie * client_cookie_in,ClientData * cd, basic_protocol::ClientRequest & request)\n",
        "SERV_NAME", sd->name(), 
        "METHOD_NAME" , method->name());
      gen_out.Print("{\n");
      gen_out.Indent();
      {
        gen_out.Print("$M_IN$ m_in;\n","M_IN",method->input_type()->name());
        gen_out.Print("$M_OUT$ m_out;\n","M_OUT",method->output_type()->name());
        gen_out.Print("bool res = m_in.ParseFromString(request.request_string());\n");
        gen_out.Print("res = impl.$M_NAME$(client_cookie_in,cd,m_in,m_out);\n","M_NAME",method->name());
// Don't need the following, the request and response objects are distinct
//        gen_out.Print("res = m_out.SerializeToString( request.mutable_response_string());\n");
      }
      gen_out.Outdent();
      gen_out.Print("}\n\n");
    }
  }

  void create_constructor(Printer & gen_out, const ServiceDescriptor * sd)
  {
    gen_out.Print("$SERV_NAME$()\n","SERV_NAME",sd->name());
    gen_out.Print("{\n");
    gen_out.Indent();
    std::map<std::string,std::string> fgw;

    for(int i=0; i < sd->method_count(); ++i)
    {
      fgw["MN"] = sd->method(i)->name();
      fgw["I"] = boost::lexical_cast<std::string>(i);
      fgw["SERV_NAME"] = sd->name();
      gen_out.Print(fgw,"m_dispatch_table.SetEntry( common::Oid(\"$MN$\",$I$), & $SERV_NAME$_$MN$<T_IMPL>);\n");
    }
    gen_out.Outdent();
    gen_out.Print("}\n");
  }
  
  void create_dispatch_function(Printer & gen_out, const ServiceDescriptor * sd)
  {
    gen_out.Print("virtual bool Dispatch(typename T_IMPL::t_client_cookie ** client_cookie,ClientData * cd, basic_protocol::ClientRequest & cr)\n");
    gen_out.Print("{\n");
    gen_out.Indent();
    {
      gen_out.Print("(*m_dispatch_table[cr.request_ordinal()])(m_impl,client_cookie,cd,cr);\n");
    }
    gen_out.Outdent();
    gen_out.Print("}\n");    
  }

  void create_service(Printer & gen_out, const ServiceDescriptor * sd)
  {
    int method_count = sd->method_count();
    gen_out.Indent();
   
    gen_out.Print("struct $SERV_NAME$_dummy_tag{};\n\n","SERV_NAME",sd->name());
    gen_out.Print("template<typename T>\n"); 
    gen_out.Print("class $SERV_NAME$_skel\n{\n","SERV_NAME",sd->name());
    {
      gen_out.Print("public:\n");
      gen_out.Indent();
        gen_out.Print("typedef ClientCookieBase t_client_cookie;\n");
        gen_out.Print("bool Init() {}\n");
        gen_out.Print("bool TearDown() {}\n");
    
        for(int i=0; i<method_count; ++i)
        {
          skel_function(gen_out,sd->method(i)); 
        }
      gen_out.Outdent();
      gen_out.Print("private:\n");
    }
    gen_out.Print("};\n\n");

    create_dispatch_functions(gen_out, sd);

    gen_out.Print("template<typename T_IMPL=$SERV_NAME$_skel<$SERV_NAME$_dummy_tag> >\n", "SERV_NAME",sd->name());
    gen_out.Print("class $SERV_NAME$ : public rubble::rpc::ServiceBase\n{\n","SERV_NAME",sd->name());
    {
      gen_out.Print("public:\n");
      gen_out.Indent();
        create_constructor(gen_out,sd);
        create_dispatch_function(gen_out,sd);
        gen_out.Print("virtual bool Init() { return m_impl.Init(); };\n");
        gen_out.Print("virtual bool TearDown() { return m_impl.TearDown(); };\n");
        gen_out.Print("virtual const char * name() { return \"$S_NAME$\"; }\n","S_NAME", sd->name());
        gen_out.Print("virtual bool require_tracking() \n");
        gen_out.Print("  { return !boost::mpl::is_void_< typename T_IMPL::t_client_cookie>::value; }\n");
      gen_out.Outdent();
        gen_out.Print("private:\n");
      gen_out.Indent();
        gen_out.Print("T_IMPL m_impl;\n");
        gen_out.Print("common::OidContainer<common::Oid,bool (*)( T_IMPL &,\n");
        gen_out.Print("                                           ClientCookie *,\n");
        gen_out.Print("                                           ClientData *,\n");
        gen_out.Print("                                           basic_protocol::ClientRequest & )\n");       
      gen_out.Print("                                           > m_dispatch_table;\n");
      gen_out.Outdent();
    }
    gen_out.Print("};\n\n");


    gen_out.Outdent();
//    {};",);
  }
}

void create_client_service( Printer & gen_out, const ServiceDescriptor * sd)
{
  int method_count = sd->method_count();
  gen_out.Indent();
  gen_out.Print("class $CN$ : public rubble::rpc::ClientServiceSubscription \n{\n", "CN", sd->name());
  gen_out.Print("public:\n");
  gen_out.Indent();
    // constructor
    gen_out.Print("$CN$()\n  : ClientServiceSubscription($MC$)\n",
      "CN",sd->name(),
      "MC",boost::lexical_cast<std::string>(method_count) );
    gen_out.Print("{\n");
    gen_out.Indent();
      for(int i=0; i<method_count; ++i)
      {
        gen_out.Print("m_service_method_map.SetEntry(common::Oid(\"$MN$\",$I$),-1);\n",
          "MN",sd->method(i)->name(),
          "I", boost::lexical_cast<std::string>(i)
          );
      }
    gen_out.Outdent();
    gen_out.Print("}\n");
      // methods
      std::map<std::string, std::string> map;
      for(int i=0; i<method_count; ++i)
      {
        map["MN"] = sd->method(i)->name();
        map["IP"] = sd->method(i)->input_type()->name();
        map["OP"] = sd->method(i)->output_type()->name();
        gen_out.Print(map,"bool $MN$(const $IP$ & req, $OP$ & res)\n{\n");
        gen_out.Indent();
          gen_out.Print("static const boost::uint16_t method_id = $I$;\n","I", boost::lexical_cast<std::string>(i));
          gen_out.Print("m_client_request.set_request_ordinal( * m_service_method_map[method_id]);\n");
          gen_out.Print("req.SerializeToString(m_client_request.mutable_request_string());\n\n");
          gen_out.Print("if(! dispatch())\n");
          gen_out.Indent();
            gen_out.Print("throw \"dispatch failed\";\n\n");
          gen_out.Outdent();
// Don't need the following, the request and response objects are distinct
//          gen_out.Print("if( !res.ParseFromString( m_client_request.response_string()))\n");
//          gen_out.Indent();
//            gen_out.Print("throw \"response string not parsing correctly\";\n\n");
          gen_out.Outdent();
        gen_out.Outdent();
        gen_out.Print("}\n");
      }
      // -- private --
  gen_out.Outdent();
  gen_out.Print("private:\n");
  gen_out.Print("};\n");
  gen_out.Outdent();
//  for(int i=0; i < sd->service_count(), ++i)

}

class RblRpcGenerator : public CppGenerator
{
  virtual bool Generate(  const FileDescriptor * file, 
                          const string & parameter, 
                          GeneratorContext * generator,
                          string * error) const
  {
      std::string stem = path(file->name()).stem().string();
      std::string stem2 = stem;

      std::string parent_path = path(file->name()).parent_path().string();

      std::string pbuf_name = parent_path.append(stem).append(".pb.h");
      int service_count = file->service_count();

      std::cerr << "RBL RPC generating filename : " << file->name() << std::endl ;
      std::cerr << "Out filename : " <<  stem << std::endl;
      std::cerr << "Service count : " << service_count << std::endl;
      
      // SERVER CODE GENERATOR //
      
      io::ZeroCopyOutputStream * stream = generator->Open(stem.append("-server.rblrpc.h"));
      stem2.append("_SERVER_RBLRPC_H");
      Printer gen_out(stream, '$');
      
      gen_out.Print("#ifndef RBL_RPC_GEN_$STEM$_SERVER_H\n", "STEM", stem2);
      gen_out.Print("#define RBL_RPC_GEN_$STEM$_SERVER_H\n\n", "STEM", stem2);

      gen_out.Print("#include \"$incl$\" \n","incl", pbuf_name);
      gen_out.Print("#include <$incl$> \n","incl", "rpc/server/ClientServiceCookies.h");
      gen_out.Print("#include <$incl$> \n","incl", "rpc/server/ServiceOracle.h");
      gen_out.Print("#include <$incl$> \n","incl", "boost/cstdint.hpp");
      gen_out.Print("#include <$incl$> \n\n","incl", "boost/mpl/void.hpp");


      std::vector<std::string> namespace_strings;
     
      typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
      boost::char_separator<char> sep(".");
      tokenizer tokens(file->package(), sep);
      for (tokenizer::iterator tok_iter = tokens.begin();
        tok_iter != tokens.end(); ++tok_iter)
          namespace_strings.push_back(*tok_iter);
      
      int ns_count = namespace_strings.size();
        for(int i=0; i < ns_count;++i)
      {
        if(i==ns_count)
          gen_out.Print("\n");
        else
        {
          gen_out.Print("namespace ");
          gen_out.Print(namespace_strings[i].c_str());
          gen_out.Print(" {\n");
        }
      }

      if(ns_count != 0)
        ;   
      {
        for(int i=0; i < service_count; ++i)
        {
          create_service(gen_out, file->service(i));
        }
      }
      if(ns_count != 0)
        for(int i=0; i <= ns_count;++i)
        {
          if(i==ns_count)
            gen_out.Print("\n");
          else if(i==0)
            gen_out.Print("}");
          else
            gen_out.Print(" }");
        }
      gen_out.Print("#endif");

      // CLIENT CODE GENERATOR // 
      stem = path(file->name()).stem().string();
      io::ZeroCopyOutputStream * stream_client = generator->Open(stem.append("-client.rblrpc.h"));
      Printer gen_out_c(stream_client, '$');
      gen_out_c.Print("#ifndef RBL_RPC_GEN_$STEM$_CLIENT_H\n", "STEM", stem);
      gen_out_c.Print("#define RBL_RPC_GEN_$STEM$_CLIENT_H\n\n", "STEM", stem);

      gen_out_c.Print("#include \"$incl$\" \n","incl", pbuf_name);
      gen_out_c.Print("#include \"$incl$\" \n","incl", "rpc/client_rpc_common.h");
      gen_out_c.Print("#include <$incl$> \n\n","incl", "boost/cstdint.hpp");

      for(int i=0; i < ns_count;++i)
      {
        if(i==ns_count)
          gen_out_c.Print("\n");
        else
        {
          gen_out_c.Print("namespace ");
          gen_out_c.Print(namespace_strings[i].c_str());
          gen_out_c.Print(" {\n");
        }
      }

      if(ns_count != 0)
        ;   
      {
        for(int i=0; i < service_count; ++i)
        {
          create_client_service( gen_out_c, file->service(i));

        }
      }
      if(ns_count != 0)
        for(int i=0; i <= ns_count;++i)
        {
          if(i==ns_count)
            gen_out_c.Print("\n");
          else if(i==0)
            gen_out_c.Print("}");
          else
            gen_out_c.Print(" }");
        }
      gen_out_c.Print("#endif\n");

      return true;
  }
};

int main(int argc, char * argv[])
{
  RblRpcGenerator rpc_generator;
  return PluginMain(argc,argv, & rpc_generator); 
}
