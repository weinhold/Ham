/*
 * Copyright 2013, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */


#include <string.h>
#include <unistd.h>

#include <map>

#include "make/MakeException.h"
#include "make/Processor.h"
#include "util/OptionIterator.h"
#include "util/TextFileException.h"


using namespace ham;


static void
print_usage(const char* programName, bool error)
{
	FILE* out = error ? stderr : stdout;
	fprintf(out, "Usage: %s [ <options> ] [ <target> ... ]\n", programName);
	fprintf(out, "Options:\n");
	fprintf(out, "  -a, --all\n");
	fprintf(out, "      Build all targets, even the ones that are "
		"up-to-date.\n");
	fprintf(out, "  -c <version>, --compatibility <version>\n");
	fprintf(out, "      Behave compatible to <version>, which is either of "
		"\"jam\" (plain Jam\n");
	fprintf(out, "      2.5), \"boost\" (Boost.Jam), or \"ham\" (Ham, the "
		"default).\n");
	fprintf(out, "  -d <option>, --debug <option>\n");
	fprintf(out, "      Enable/set a debug option. Options are:\n");
	fprintf(out, "      a     -  Print the actions\n");
	fprintf(out, "      c     -  Print the causes\n");
	fprintf(out, "      d     -  Print the dependencies\n");
	fprintf(out, "      m     -  Print the make tree\n");
	fprintf(out, "      x     -  Print the make commands\n");
	fprintf(out, "      0..9  -  Set debug level.\n");
	fprintf(out, "  -f <file>, --jambase <file>\n");
	fprintf(out, "      Execute <file> instead of the built-in Jambase.\n");
	fprintf(out, "  -g, --from-newest\n");
	fprintf(out, "      Build from the newest sources first.\n");
	fprintf(out, "  -h, --help\n");
	fprintf(out, "      Print this usage message.\n");
	fprintf(out, "  -j <jobs>, --jobs <jobs>\n");
	fprintf(out, "      Use up to <jobs> number of concurrent shell "
		"processes.\n");
	fprintf(out, "  -n, --dry-run\n");
	fprintf(out, "      Print actions and commands, but don't run them.\n");
	fprintf(out, "  -o <file>, --actions <file>\n");
	fprintf(out, "      Write the actions to file <file>.\n");
	fprintf(out, "  -q, --quit-on-error\n");
	fprintf(out, "      Quit as soon as making a target fails.\n");
	fprintf(out, "  -s <variable>=<value>, --set <variable>=<value>\n");
	fprintf(out, "      Set variable <variable> to <value>, overriding the "
		"environmental variable.\n");
	fprintf(out, "  -t <target>, --target <target>\n");
	fprintf(out, "      Rebuild target <target>, even if it is up-to-date.\n");
	fprintf(out, "  -v, --version\n");
	fprintf(out, "      Print the Ham version and exit.\n");
}


static void
print_usage_end_exit(const char* programName, bool error)
{
	print_usage(programName, error);
	exit(error ? 1 : 0);
}


static bool
set_variable(std::map<data::String, data::String>& variables,
	const char* variable)
{
	const char* equalSign = strchr(variable, '=');
	if (equalSign == NULL)
		return false;

	variables[data::String(variable, equalSign - variable)]
		= data::String(equalSign + 1);
	return true;
}


int
main(int argc, const char* const* argv)
{
	const char* programName = argc >= 1 ? argv[0] : "ham";

	// get standard variables
	std::map<data::String, data::String> variables;
	set_variable(variables, "UNIX=true");
	set_variable(variables, "OS=LINUX");
	set_variable(variables, "OSPLAT=X86");
	set_variable(variables, "JAMVERSION=2.5-haiku-20111222");

	// import environment
// TODO: Platform specific!
	for (size_t i = 0; environ[i] != NULL; i++)
		set_variable(variables, environ[i]);

	// parse arguments
	std::string jambaseFile;
	bool jambaseFileSpecified = false;
	std::string actionsOutputFile;
	bool actionsOutputFileSpecified = false;
	behavior::Compatibility compatibility = behavior::COMPATIBILITY_HAM;
	bool compatibilitySpecified = false;
	bool buildFromNewest = false;
	int jobCount = 1;
	bool dryRun = false;
	bool quitOnError = false;
	bool printMakeTree = false;
	bool printActions = false;
	bool printCommands = false;
	data::StringList forceUpdateTargets;

	util::OptionIterator optionIterator(argc, argv,
		util::OptionSpecification()
			.Add('a', "--all")
			.Add('c', "--compatibility", true)
			.Add('d', "--debug", true)
			.Add('f', "--jambase", true)
			.Add('g', "--from-newest")
			.Add('h', "--help")
			.Add('j', "--jobs", true)
			.Add('n', "--dry-run")
			.Add('o', "--actions", true)
			.Add('q', "--quit-on-error")
			.Add('s', "--set", true)
			.Add('t', "--target", true)
			.Add('v', "--version")
	);

	while (optionIterator.HasNext()) {
		// short ("-") option(s)
		std::string argument;
		switch (optionIterator.Next(argument)) {
			case 'c':
			{
				if (argument == "jam") {
					compatibility = behavior::COMPATIBILITY_JAM;
				} else if (argument == "boost") {
					compatibility = behavior::COMPATIBILITY_BOOST_JAM;
				} else if (argument == "ham") {
					compatibility = behavior::COMPATIBILITY_HAM;
				} else {
					fprintf(stderr, "Error: Invalid argument for "
						"compatibility option: \"%s\"\n", argument.c_str());
					exit(1);
				}
				compatibilitySpecified = true;
				break;
			}

			case 'd':
			{
				if (argument.empty())
					print_usage_end_exit(programName, true);
				for (size_t i = 0; i < argument.length(); i++) {
					switch (argument[i]) {
						case 'a':
							printActions = true;
							break;
// TODO:...
//						case 'c':
//							printCauses = true;
//							break;
//						case 'd':
//							printDependencies = true;
//							break;
						case 'm':
							printMakeTree = true;
							break;
						case 'x':
							printCommands = true;
							break;
						default:
// TODO: 0-9
							print_usage_end_exit(programName, true);
					}
				}
				break;
			}

			case 'f':
				jambaseFile = argument;
				jambaseFileSpecified = true;
				break;

			case 'g':
				buildFromNewest = true;
				break;

			case 'h':
				print_usage_end_exit(programName, false);

			case 'j':
			{
				char* end;
				jobCount = strtol(argument.c_str(), &end, 0);
				if (*end != '\0')
					print_usage_end_exit(programName, true);
				break;
			}

			case 'n':
				dryRun = true;
				break;

			case 'o':
				actionsOutputFile = argument;
				actionsOutputFileSpecified = true;
				break;

			case 'q':
				quitOnError = true;
				break;

			case 's':
			{
				if (!set_variable(variables, argument.c_str()))
					print_usage_end_exit(programName, true);
				break;
			}

			case 't':
				forceUpdateTargets.Append(String(argument.c_str()));
				break;

			case 'v':
				printf("Ham 0.1, Copyright 2010-2013 Ingo Weinhold.");
				exit(0);

			default:
				print_usage_end_exit(programName, true);
		}
	}

	if (optionIterator.ErrorOccurred())
		print_usage_end_exit(programName, true);

	// get targets to be made
	StringList primaryTargets;
	for (int i = optionIterator.Index(); i < argc; i++)
		primaryTargets.Append(String(argv[i]));

	// "all" is the default target, if none is given.
	if (primaryTargets.IsEmpty())
		primaryTargets.Append("all");

	// dry-run implies printing actions and command, unless one or both of
	// those have been specified explicitly.
	if (dryRun && !printActions && ! printCommands) {
		printActions = true;
		printCommands = true;
	}

	make::Processor processor;
// TODO: Add environmental variables!

	// set explicitly specified variables
	for (std::map<data::String, data::String>::iterator it = variables.begin();
		it != variables.end(); ++it) {
		data::StringList value;
		value.Append(it->second);
		processor.GlobalVariables().Set(it->first, value);
	}

	// Set compatibility. If not specified explicitly, infer from the program
	// name.
	if (!compatibilitySpecified) {
		const char* slash = strrchr(programName, '/');
		const char* baseName = slash != NULL ? slash + 1 : programName;
		if (strcmp(baseName, "jam") == 0)
			compatibility = behavior::COMPATIBILITY_JAM;
		else if (strcmp(baseName, "bjam") == 0)
			compatibility = behavior::COMPATIBILITY_BOOST_JAM;
	}

	processor.SetCompatibility(compatibility);

	// set other options
	processor.SetPrimaryTargets(primaryTargets);
	if (jambaseFileSpecified)
		processor.SetJambaseFile(jambaseFile.c_str());
	processor.SetBuildFromNewest(buildFromNewest);
	processor.SetJobCount(jobCount);
	processor.SetDryRun(dryRun);
	processor.SetPrintMakeTree(printMakeTree);
	processor.SetPrintActions(printActions);
	processor.SetPrintCommands(printCommands);
	if (actionsOutputFileSpecified)
		processor.SetActionsOutputFile(actionsOutputFile.c_str());
	processor.SetQuitOnError(quitOnError);

	try {
		// execute the jam code
		processor.ProcessJambase();

		// Set the targets that shall be made, even if up-to-date.
		processor.SetForceUpdateTargets(forceUpdateTargets);

		// prepare the targets
		processor.PrepareTargets();
	} catch (make::MakeException& exception) {
		fprintf(stderr, "%s.\n", exception.Message());
	} catch (util::TextFileException& exception) {
		const util::TextFilePosition& position = exception.Position();
		fprintf(stderr, "%s:%zu:%zu: %s.\n", position.FileName(),
			position.Line() + 1, position.Column() + 1, exception.Message());
	}

// TODO: Catch exceptions...

	return 0;
}
