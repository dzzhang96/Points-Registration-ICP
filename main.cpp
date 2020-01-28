#include "Registration.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	Registration w;
	w.show();
	return a.exec();
}
