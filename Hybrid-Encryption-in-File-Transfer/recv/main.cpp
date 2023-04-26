#include "recvfileop.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	recvfileop w;
	w.show();
	return a.exec();
}
