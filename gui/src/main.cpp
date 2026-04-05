#include "orderbook_widget.h"

#include <QApplication>

int main(int argc, char * argv[])
{
    QApplication app(argc, argv);
    OrderBookWidget orderBookWgt;
    orderBookWgt.show();
    return app.exec();
}
