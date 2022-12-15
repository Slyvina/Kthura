// Lic:
// ***********************************************************
// Kthura/Kthura2XML/Kthura2XML.cpp
// This particular file has been released in the public domain
// and is therefore free of any restriction. You are allowed
// to credit me as the original author, but this is not
// required.
// This file was setup/modified in:
// 2022
// If the law of your country does not support the concept
// of a product being released in the public domain, while
// the original author is still alive, or if his death was
// not longer than 70 years ago, you can deem this file
// "(c) Jeroen Broks - licensed under the CC0 License",
// with basically comes down to the same lack of
// restriction the public domain offers. (YAY!)
// ***********************************************************
// Version 22.12.15
// EndLic
#include <SlyvString.hpp>
#include <SlyvStream.hpp>
#include <Kthura_Core.hpp>
#include <Kthura_Export_XML.hpp>

// Now don't expect TOO MUCH from this program
// I didn't create it to make it all possible to export Kthura to XML
// It's merely a debug program and the XML output is easy to read, so that can help me to see
// if stuff works the way it should... Well, at least until I can actually do some
// graphic representation of a Kthura Map. Sorry to disappoint ya!

using namespace std;
using namespace Slyvina::Units;
using namespace Slyvina::Kthura;

int main(int argc, char** arg) {
	cout << "Kthura 2 XML\n\nCoded by: Jeroen P. Broks\n\nThis particular program has been released in the public domain.\nPlease note that the libraries used are copyrighted!\n\n";
	if (argc < 3) {
		//cout << argc << "\n"; //debug
		cout << "Usage: " << StripAll(arg[0]) << " <input map file> <output xml file>\n";
		return 0;
	}
	if (!FileExists(arg[1])) { cout << "Input file '" << arg[1] << "' not found!\n\n";	return 404; }
	cout << "Loading: " << arg[1] << endl;
	auto KMap = LoadKthura(arg[1]);
	cout << "Saving:  " << arg[2] << endl;
	Kthura_Export_XML(KMap, arg[2]);
	cout << "Ok\n\n";
}