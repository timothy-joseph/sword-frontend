/* commands of the loop include:
 * TODO:
 * ?        -> print commentary
 * /<regex> -> search for verses which contain <regex>
 *
 *
 * DONE:
 * .        -> print verse
 * n 		-> increment verse by one
 * N		-> decrement verse by one
 * xn 		-> increment verse by x
 * xN 		-> decrement verse by x
 * P 		-> prints the whole capitol
 * ~		-> gets you to book selection
 * q 		-> quit
 *
 * 	entry sequence;
 * 	render translations, render book names, *render capitols number, *render verse number;
 */
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <regex.h>

#include <swmgr.h>
#include <swmodule.h>
#include <swfilter.h>
#include <markupfiltmgr.h>
#include <versekey.h>
#include <rawtext.h>

#include "render.h"

/* macros */
using namespace sword;
using namespace std;

using sword::SWMgr;
using sword::MarkupFilterMgr;
using sword::SWModule;
using sword::FMT_WEBIF;
using sword::FMT_HTMLHREF;
using sword::FMT_XHTML;
using sword::FMT_PLAIN;
using sword::FMT_RTF;
using sword::FMT_LATEX;
using sword::ModMap;
using sword::AttributeTypeList;
using sword::AttributeList;
using sword::AttributeValue;
using sword::VerseKey;
using sword::FilterList;

/* functions declarations */
int untilletter(string str);
void stdinoptions(void);
void increment(int n);
void decrement(int n);
void renderverse(void);
void rendercom(void);
void findcap(string pat);
void loop(void);

/* global variables */
SWMgr manager(new MarkupFilterMgr(FMT_PLAIN));
SWModule *target;
SWModule *targetcom;
VerseKey *vk;
VerseKey *vkcom;

char printed = 0;

/* function definitions */
int
untilletter(string str)
{
	int num = 0;

	string::iterator it;
	for (it = str.begin(); it != str.end(); it++)
		if (isdigit(*it))
			num = num * 10 + (*it - '0');
		else
			break;

	if (*it == 'n') {
		increment(num);
		return 0;
	} else if (*it == 'N') {
		decrement(num);
		return 0;
	}

	return num;
}

void
stdinoptions(void)
{
	string translation;
	string bookname;
	string cap;
	string verse;
	string keystr;

	/* get the translation */
	cout << "Available Translations: ";
	renderModuleNames();
	cout << "Choose a translation: ";
	cin >> translation;
	/* get the book name */
	cout << "Books: ";
	renderBiblicalBooks();
	cout << "Choose a book: ";
	cin >> bookname;
	/* get the cap */
	cout << "Choose a cap: ";
	cin >> cap;
	/* get the verse */
	cout << "Choose a verse: ";
	cin >> verse;

	keystr = bookname + " " + cap + ":" + verse;

	/* now select the target */
	target = manager.getModule(translation.c_str());
	if (!target) {
		fputs(translation.c_str(), stderr);
		fputs(" is not a valid translation\n", stderr);
		exit(-1);
	}

	vk = dynamic_cast<VerseKey *>(target->getKey());
	if (vk) {
		vk->setIntros(true);
		vk->setText(keystr.c_str());
	}
	else {
		exit(-1);
	}
}

void
increment(int n)
{
	int i;

	for (i = 1; i <= n; i++) {
		vk->increment(1);
		if (targetcom)
			vkcom->increment(1);
		renderverse();
	}
}

void
decrement(int n)
{
	int i;

	for (i = 1; i <= n; i++) {
		vk->decrement(1);
		if (targetcom)
			vkcom->decrement(1);
		renderverse();
	}
}

void
renderverse(void)
{
	target->renderText();		// force an entry lookup to resolve key to something in the index

	cout << target->getKeyText() << ": ";
	cout << target->renderText();
	cout << endl;
}

void
rendercom(void)
{
	targetcom->renderText();

	cout << targetcom->getKeyText() << ": ";
	cout << targetcom->renderText();
	cout << endl;
}

void
rendercap(void)
{
	int chapter = vk->getChapter();
	vk->setVerse(1);

	while (vk->getChapter() == chapter) {
		renderverse();
		vk->increment(1);
	}

	vk->decrement(1);
}

void
percentUpdate(char percent, void *userData)
{
	char maxHashes = *((char *)userData);
	
	while ((((float)percent)/100) * maxHashes > printed) {
		std::cerr << "=";
		printed++;
		std::cerr.flush();
	}
	std::cout.flush();
}

void
findcaps(string pat)
{
	char SEARCH_TYPE = 0;
	int flags = 0 | REG_ICASE;

	ListKey listkey;
	ListKey *scope = 0;

	SWBuf searchTerm = pat.c_str();
	cerr << "[0=================================50===============================100]\n ";
	char lineLen = 70;
	printed = 0;

	listkey = target->search(searchTerm.c_str(), SEARCH_TYPE, flags, scope, 0, &percentUpdate, &lineLen);
	cout << endl;

	while (!listkey.popError()) {
		std::cout << (const char *)listkey;
		if (listkey.getElement()->userData) std::cout << " : " << (__u64)listkey.getElement()->userData << "%";
		cout << ", ";
		listkey++;
	}

	cout << endl;
}

void
loop(void)
{
	string command;
	int amount;
	while (command != "q") {
		cout << "command: ";
		cin >> command;

		if (command.length() == 0)
			continue;

		/* determine command */
		if (command == ".")
			renderverse();
		else if (command == "?")
			rendercom();
		else if (command == "~")
			stdinoptions();
		else if (command == "n")
			increment(1);
		else if (command == "N")
			decrement(1);
		else if (isdigit(command[0]))
			untilletter(command);
		else if (command == "P")
			rendercap();
		else if (command[0] == '/')
			findcaps(command.substr(1));
	}
}

int
main(int argc, char *argv[])
{
	if (argc == 1) {
		stdinoptions();
	} else {
		target = manager.getModule(argv[1]);
		if (!target)
			exit(-1);

		vk = dynamic_cast<VerseKey *>(target->getKey());
		if (vk) {
			vk->setIntros(true);
			vk->setText(argv[2]);
		}
		else {
			exit(-1);
		}
	}

	if (!(argc == 4 && argv[3][0] == 'n')) {
		char answer;
		/* TODO: ask user if he wants commentary */
		cout << "Do you want a commentary (y/n) [n]:";
		cin >> answer;

		if (answer == 'y') {
			cout << "commentaries: ";
			renderModuleNames(SWMgr::MODTYPE_COMMENTARIES);
			string choice;
			cin >> choice;
			targetcom = manager.getModule(choice.c_str());

			if (!targetcom)
				exit(-1);

			vkcom = dynamic_cast<VerseKey *>(targetcom->getKey());
			if (vkcom) {
				vkcom->setIntros(true);
				vkcom->setText(vk->getText());
			}
			else {
				exit(-1);
			}
		}
	}

	renderverse();
	loop();

	return 0;
}
