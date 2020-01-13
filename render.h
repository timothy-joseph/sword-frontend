#include <iostream>

#include <swmgr.h>
#include <swmodule.h>
#include <swconfig.h>


using namespace sword;
using namespace std;

void
renderModuleNames(const char *modType = SWMgr::MODTYPE_BIBLES)
{
	SWMgr library;
	ModMap::iterator it;

	for (it = library.Modules.begin(); it != library.Modules.end(); ++it) {
		SWModule *module = (*it).second;
		if (!strcmp(module->getType(), modType)) {
			cout << module->getName() << '\t';
		}
	}

	cout << std::endl;
}

void
renderBiblicalBooks(void)
{
		VerseKey v;
		SWConfig config("/usr/share/sword/locales.d/abbr.conf");

		for (int b = 0; b < 2; b++) {

			if (b == 0)
				cout << endl << endl << "Old Testament" << endl;
			else
				cout << endl << endl << "New Testament" << endl;

			v.setTestament(b+1);
			for (int i = 0; i < v.BMAX[b]; i++) {
				v.setBook(i+1);
				cout << v.getBookName() << "(" << config["Text"][v.getBookName()] << "), ";
			}
		}

		cout << endl;
}
