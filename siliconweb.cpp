/**
*************************************************************
* @file siliconweb.cpp
* @brief Silicon Web extensions
* Functions and Keywords intended to be used in web templates
*
* Keywords may be used for input/output of global template data.
*
* @author Gaspar Fernández <blakeyed@totaki.com>
* @version 0.1
* @date 28 sep 2015
*
* Changelog:
*
*
*
*************************************************************/

/*
  Used keywords:
   _siliconWeb = "1": SiliconWeb is loaded (global)
   _baseURL         : URL with http:// to use as base path for all resources
   _cssURL          : _baseURL/_cssURL/ is where all css are stored
   _jsURL           : _baseURL/_jsURL/ is where all js are stored
   _renderResources : CSS / JS will be rendered directly or by calling
                      "renderCSS" / "renderJS" function. "0" is false,
		      otherwise, true

  Used collections:
  _CSS              : To store all CSS files to be rendered
  _JS               : To store all JS files to be rendered
  _directJS         : To store all JS code to be attached

  Available functions (for templates):
  includeCss ( file="cssfile" [media="media"] ) : include css file
  includeJs  ( file="jsfile" )                  : include js file
  extraJs (data = extra javascript)
  renderCss
  renderJs
  list : renders a list with a collection

  Available methods (for C++):
  */
#include "siliconweb.h"

namespace
{
  std::string addSlash(std::string path)
  {
    return ( (!path.empty()) && (path.back()!='/') )?path+'/':path;
  }
}

std::string SiliconWeb::_defaultUrl ="";
std::string SiliconWeb::_cssUrl ="";
std::string SiliconWeb::_jsUrl ="";
bool SiliconWeb::_renderDefault = true;

void SiliconWeb::load(Silicon * s)
{
  Silicon::setGlobalKeyword("_siliconWeb", "1");

  loadFunction("includeCss", SiliconWeb::includeCss, s);
  loadFunction("includeJs", SiliconWeb::includeJs, s);
  loadFunction("directJs", SiliconWeb::directJs, s);
  loadFunction("renderCss", SiliconWeb::renderCss, s);
  loadFunction("renderJs", SiliconWeb::renderJs, s);
  loadFunction("list", SiliconWeb::list, s);
}

std::string SiliconWeb::list (Silicon* s, Silicon::StringMap args, std::string input)
{
  auto collection = args.find("collection");
  if (collection == args.end())
    return "";
  std::string col = collection->second;

  std::string templ = "<ul>\n"
    "{%collection var="+col+"}}\n"
    "<li>"+col+".text</li>\n"
    "{/collection}}\n"
    "</ul>";
  return s->parse(templ);
}

std::string SiliconWeb::renderCss (Silicon* s, Silicon::StringMap args, std::string input)
{
  auto list = s->getCollection("_CSS");
  if (list.empty())
    return "";

  auto comments = args.find("comments");
  bool printComment = ( (comments != args.end()) && (comments->second != "0") );

  std::string out;
  if (printComment)
    out+="<!-- Start styles -->\n";

  for (auto s : list)
    {
      auto code = s.find("code");
      out+=code->second+"\n";
    }

  if (printComment)
    out+="<!-- End styles -->\n";

  return out;
}

std::string SiliconWeb::includeCss (Silicon* s, Silicon::StringMap args, std::string input)
{
  std::string out;
  auto file=args.find("file");
  if (file==args.end())
    return "";

  auto media=args.find("media");
  out= "<link href=\""+getCssUrl(s)+file->second+"\" rel=\"stylesheet\" type=\"text/css\"";
  if (media!=args.end())
    out+=" media=\""+media->second+"\"";
  out+=" />";

  if (getDoRender(s))
    return out;
  else
    s->addToCollection("_CSS", { { "file", file->second }, { "href", getCssUrl(s)+file->second }, { "media", media->second }, { "code", out } });

  return "";
}

std::string SiliconWeb::getBaseUrl(Silicon * s)
{
  std::string baseURL;
  bool havePath = s->getKeyword("_baseURL", baseURL);
  return (havePath)?addSlash(baseURL):addSlash(defaultUrl());
}

std::string SiliconWeb::getCssUrl(Silicon * s)
{
  std::string cssURL;
  bool haveCssUrl = s->getKeyword("_cssURL", cssURL);
  if (!haveCssUrl)
    cssURL = cssUrl();

  std::string basePath = getBaseUrl(s);
  if ( (basePath.empty()) && (cssURL.empty()) )
    return "";

  if (cssURL.front()=='/')
    cssURL.substr(1);

  return addSlash(basePath+cssURL);
}

bool SiliconWeb::getDoRender(Silicon * s)
{
  auto render = s->getKeyword("_renderResources");
  return (render.empty())?_renderDefault:(render!="0");
}

std::string SiliconWeb::includeJs(Silicon * s, Silicon :: StringMap args, std :: string input)
{
  std::string out;
  auto file=args.find("file");
  if (file==args.end())
    return "";

  out= "<script type=\"text/javascript\" src=\""+getJsUrl(s)+file->second+"\" rel=\"stylesheet\"></script>";

  if (getDoRender(s))
    return out;
  else
    s->addToCollection("_JS", { { "file", file->second }, { "src", getJsUrl(s)+file->second }, { "code", out } });

  return "";
}

std::string SiliconWeb::directJs(Silicon * s, Silicon :: StringMap args, std :: string input)
{
  if (input.empty())
    return "";

  s->addToCollection ("_directJS", { { "code", input } });

 return "";
}

std::string SiliconWeb::renderJs(Silicon * s, Silicon :: StringMap args, std :: string input)
{
  std::string out;

  auto comments = args.find("comments");
  bool printComment = ( (comments != args.end()) && (comments->second != "0") );
  if (printComment)
    out+="<!-- Start scripts -->\n";

  auto test = args.find("files");
  bool renderFiles = ( (test == args.end()) || (test->second!="0") );
  test = args.find("direct");
  bool renderDirect = ( (test == args.end()) || (test->second!="0") );

  if (renderFiles)
    {
      auto list = s->getCollection("_JS");
      if (!list.empty())
	{
	  for (auto s : list)
	    {
	      auto code = s.find("code");
	      out+=code->second+"\n";
	    }
	}
    }
  if (renderDirect)
    {
      auto list = s->getCollection("_directJS");
      if (!list.empty())
	{
	  out+="<script type=\"text/javascript\">";
	  for (auto s : list)
	    {
	      auto code = s.find("code");
	      out+=code->second+"\n";
	    }
	  out+="</script>\n";
	}
    }
  if (printComment)
    out+="<!-- End scripts -->\n";

  return out;
}

std::string SiliconWeb::getJsUrl(Silicon * s)
{
  std::string jsURL;
  bool haveJsUrl = s->getKeyword("_jsURL", jsURL);
  if (!haveJsUrl)
    jsURL = jsUrl();

  std::string basePath = getBaseUrl(s);
  if ( (basePath.empty()) && (jsURL.empty()) )
    return "";

  if (jsURL.front()=='/')
    jsURL.substr(1);

  return addSlash(basePath+jsURL);
}
