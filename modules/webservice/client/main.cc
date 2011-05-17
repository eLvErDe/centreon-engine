/*
** Copyright 2011 Merethis
**
** This file is part of Centreon Engine.
**
** Centreon Engine is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Engine is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Engine. If not, see
** <http://www.gnu.org/licenses/>.
*/

#include <limits.h>
#if defined(__GNU_LIBRARY__) || defined(__GLIBC__) // Version 5 and 6, respectively.
# include <getopt.h>
# define HAVE_GETOPT_LONG
#endif /* GLIBC */

#include <QHash>
#include <QString>
#include <iostream>
#include <exception>
#include <stdlib.h>
#include <signal.h>

#include "soapH.h"
#include "centreonengine.nsmap" // gSOAP namespaces.
#include "auto_gen.hh"
#include "webservice.hh"

using namespace com::centreon::engine::modules::client;

static char const* OPTIONS = "a:c:e:f:hk:lp:s";

/**
 *  This function show the application's usage.
 *
 *  @param[in] appname The application name.
 */
static void usage(char const* appname) {
  std::cout << "usage: " << appname
#ifdef WITH_OPENSSL
	    << " [-a action] [-e end_point] [-h] [-l] [-s [-c cacert] [-k keyfile] [-p password]] function [arg ...]" << std::endl
#else
	    << " [-a action] [-e end_point] [-h] [-l] function [arg ...]" << std::endl
#endif // !WITH_OPENSSL
	    << " -a, --action    override the default SOAP action. " << std::endl
	    << " -e, --end_point override the default SOAP service location." << std::endl
	    << " -f, --function  the function name to call." << std::endl
	    << " -h, --help      this help." << std::endl
	    << " -l, --list      list all prototype." << std::endl;
#ifdef WITH_OPENSSL
  std::cout << " -c, --cacert    optional cacert file to store trusted certificates. " << std::endl
	    << " -d, --dh        DH file name or DH key len bits to generate DH param. " << std::endl
	    << " -k, --keyfile   required when server must authenticate to clients. " << std::endl
	    << " -p, --password  password to read the key file. " << std::endl
	    << " -s, --ssl       enable ssl. " << std::endl;
#endif // !WITH_OPENSSL
  exit(EXIT_FAILURE);
}

/**
 *  This function show all function prototype.
 */
static void show_prototype() {
  std::cout << "list all prototype:" << std::endl;
  auto_gen::instance().show_help();
  exit(EXIT_SUCCESS);
}

/**
 *  Parse the command line options.
 *
 *  @param[out] args Options list that will be filled.
 *  @param[in]  argc Number of arguments.
 *  @param[in]  argv Arguments value.
 *
 *  @return The function arguments.
 */
static QList<QString> parse_option(QHash<char, QString>& opt, int argc, char** argv) {
#ifdef HAVE_GETOPT_LONG
  static struct option const long_opt[] = {
    { "action",    required_argument, NULL, 'a' },
    { "end_point", required_argument, NULL, 'e' },
    { "function",  required_argument, NULL, 'f' },
    { "help",      no_argument,       NULL, 'h' },
    { "list",      no_argument,       NULL, 'l' },
# ifdef WITH_OPENSSL
    { "keyfile",   required_argument, NULL, 'k' },
    { "cacert",    required_argument, NULL, 'c' },
    { "password",  required_argument, NULL, 'p' },
    { "ssl",       no_argument,       NULL, 's' },
# endif // !WITH_OPENSSL
    { NULL,        0,                 NULL, 0 }
  };
#endif /* !HAVE_GETOPT_LONG */
  char const* appname = argv[0];
  char c;

  opt['e'] = "127.0.0.1:4242";
  opt['s'] = "false";

#ifdef HAVE_GETOPT_LONG
  while ((c = getopt_long(argc, argv, OPTIONS, long_opt, NULL)) != -1) {
#else
  while ((c = getopt(argc, argv, OPTIONS)) != -1) {
#endif /* !HAVE_GETOPT_LONG */
    switch (c) {
    case 'a':
    case 'e':
    case 'f':
#ifdef WITH_OPENSSL
    case 'c':
    case 'k':
    case 'p':
#endif // !WITH_OPENSSL
      opt[c] = optarg;
      break;

    case 'l':
      show_prototype();
      break;

#ifdef WITH_OPENSSL
    case 's':
      opt[c] = "true";
      break;
#endif // !WITH_OPENSSL

    case 'h':
    default:
      usage(appname);
    }
  }

  if (optind == argc
      || (opt['s'] == "false"
	  && (opt['c'] != ""
	      || opt['d'] != ""
	      || opt['k'] != ""
	      || opt['p'] != ""))) {
    usage(appname);
  }

  if (opt['f'] == "") {
    opt['f'] = argv[optind++];
  }

  QList<QString> args;
  for (int i = optind; i < argc; ++i) {
    args.push_back(argv[i]);
  }

  return (args);
}

int main(int argc, char** argv) {
  int ret = EXIT_SUCCESS;

  try {
    QHash<char, QString> opt;
    QList<QString> args = parse_option(opt, argc, argv);

    if (args.size() == 1 && args.front() == "help") {
      std::cout << "usage: " << argv[0] << " ";
        auto_gen::instance().show_help(opt['f']);
    }
    else {
      webservice ws(opt['s'] == "true", opt['k'], opt['p'], opt['c']);
      ws.set_end_point(opt['e']);
      ws.set_action(opt['a']);

      ret = (ws.execute(opt['f'], args) == true ? EXIT_SUCCESS : EXIT_FAILURE);
    }
  }
  catch (std::exception const& e) {
    std::cerr << "error: " << e.what() << std::endl;
    return (EXIT_FAILURE);
  }
  catch (...) {
    std::cerr << "error: catch all." << std::endl;
    return (EXIT_FAILURE);
  }
  return (ret);
}