#include "orderbook_widget.h"

#include <QBoxLayout>
#include <QSplitter>

//=================================================
//=================================================

OrderBookWidget::OrderBookWidget(QWidget * parent)
    : QMainWindow{parent}
    , m_pauseSimulation{true}
    , m_marketPrice{0.0}
{
    // Window title (on the frame)
    setWindowTitle("Order book");

    // Configure interface for the order book

    mp_asksTblWgt = new QTableWidget;
    mp_asksTblWgt->setColumnCount(2);
    mp_asksTblWgt->setHorizontalHeaderLabels({"Price", "Volume"});
    mp_asksTblWgt->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mp_asksTblWgt->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    mp_bidsTblWgt = new QTableWidget;
    mp_bidsTblWgt->setColumnCount(2);
    mp_bidsTblWgt->setHorizontalHeaderLabels({"Price", "Volume"});
    mp_bidsTblWgt->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mp_bidsTblWgt->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    mp_marketPriceLbl = new QLabel;
    mp_spreadLbl = new QLabel;

    auto bookHBoxLayout = new QHBoxLayout;
    auto bookVBoxLayout = new QVBoxLayout;

    bookHBoxLayout->addWidget(mp_marketPriceLbl);
    bookHBoxLayout->addWidget(mp_spreadLbl);

    bookVBoxLayout->addWidget(mp_asksTblWgt);
    bookVBoxLayout->addLayout(bookHBoxLayout);
    bookVBoxLayout->addWidget(mp_bidsTblWgt);

    auto bookPartWgt = new QWidget;
    bookPartWgt->setLayout(bookVBoxLayout);

    // Configure interface for the DOM (Depth of Market)

    mp_DOMCustomPlot = new QCustomPlot;

    // Set the splitter between the book and the DOM

    auto mainSplitter = new QSplitter;
    mainSplitter->addWidget(mp_DOMCustomPlot);
    mainSplitter->addWidget(bookPartWgt);

    // Last display settings

    setCentralWidget(mainSplitter);
    setMinimumSize(300, 300);
    resize(1000, 500);
    mainSplitter->setSizes({500, 500});
}

//=================================================

OrderBookWidget::~OrderBookWidget()
{
}
