#include "orderbook_widget.h"

#include <QBoxLayout>
#include <QSplitter>
#include <QMenuBar>
#include <QMenu>
#include <QAction>

#include <random>

Q_DECLARE_METATYPE(PriceLevelsVector)

//=================================================
//=================================================

void TradingSimulationThread::finish_request()
{
    m_finish = true;
}

//=================================================

void TradingSimulationThread::run()
{
    // Codes of the actions over the order book
    enum class Action {ADD = 0, DLT = 1, MDF = 2, EXEC = 3};

    // Each action as well as parameters for actions are generated randomly
    std::random_device rd;
    std::mt19937 gen(rd());

    // ADD - 0; DELETE - 1; MODIFY - 2; EXECUTE - 3
    std::uniform_int_distribution<> actionDist(0, 3);

    // ASK - 0; BID - 1
    std::uniform_int_distribution<> typeDist(0, 1);

    // Order price and volume
    std::uniform_real_distribution<> priceDist(1, 100);
    std::uniform_int_distribution<> volumeDist(1, 10);

    for (OrderID currId = 0; currId < SIZE_MAX && m_finish == false;)
    {
        if (currId < 5) // initial addition of five asks
        {
            m_book.add_order(currId++, 100.0 - currId * 10, volumeDist(gen), OrderType::ASK);
        }
        else if (currId < 10) // initial addition of five bids
        {
            m_book.add_order(currId++, 100.0 - currId * 10, volumeDist(gen), OrderType::BID);
        }
        else // subsequent iterations
        {
            // Generate the action to perform over the order book
            auto action = static_cast<Action>(actionDist(gen));

            switch (action)
            {
            case Action::ADD:
            {
                m_book.add_order(currId++,
                                 priceDist(gen),
                                 volumeDist(gen),
                                 static_cast<OrderType>(typeDist(gen)));
                break;
            }
            case Action::DLT:
            {
                if (currId == 0) // no orders were added yet
                    continue;
                std::uniform_int_distribution<> idDist(0, currId - 1);
                m_book.delete_order(idDist(gen));
                break;
            }
            case Action::MDF:
            {
                if (currId == 0) // no orders were added yet
                    continue;
                std::uniform_int_distribution<> idDist(0, currId - 1);
                m_book.modify_order(idDist(gen), priceDist(gen), volumeDist(gen));
                break;
            }
            case Action::EXEC:
            {
                m_book.execute(static_cast<OrderType>(typeDist(gen)), volumeDist(gen));
                break;
            }
            }
        }

        // Notify the main thread that the book was updated
        emit book_updated(m_book.price_levels_for_type(OrderType::ASK),
                          m_book.price_levels_for_type(OrderType::BID),
                          m_book.market_price(),
                          m_book.spread());

        // Little delay before the next action
        QThread::sleep(1);
    }
}

//=================================================
//=================================================

OrderBookWidget::OrderBookWidget(QWidget * parent)
    : QMainWindow{parent}
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

    // Register the complex type so that objects of that type are able
    // to be transferred using the signal/slot mechanism
    qRegisterMetaType<PriceLevelsVector>();

    // Create and configure a menu bar

    auto menuBar = new QMenuBar;
    setMenuBar(menuBar);

    auto launchMenu = menuBar->addMenu("&Launch");
    auto simulationMenu = launchMenu->addMenu("Simulation");

    mp_startSimuAct = simulationMenu->addAction("Start Simulation", this,
                                                [this] ()
                                                {
                                                    this->mp_startSimuAct->setDisabled(true);
                                                    this->mp_stopSimuAct->setEnabled(true);
                                                    this->create_configure_start_thread();
                                                });

    mp_stopSimuAct = simulationMenu->addAction("Stop Simulation", this,
                                               [this] ()
                                               {
                                                   this->mp_startSimuAct->setEnabled(true);
                                                   this->mp_stopSimuAct->setDisabled(true);
                                                   emit finish_simulation();
                                                   mp_tradingSimuThread->wait();
                                               });

    mp_startSimuAct->setEnabled(true);
    mp_stopSimuAct->setDisabled(true);

    // Last display settings

    setCentralWidget(mainSplitter);
    setMinimumSize(300, 300);
    resize(1000, 500);
    mainSplitter->setSizes({500, 500});
}

//=================================================

OrderBookWidget::~OrderBookWidget()
{
    // Stop the simulation thread and wait until it completely terminates
    emit finish_simulation();
    mp_tradingSimuThread->wait();
}

//=================================================

void OrderBookWidget::create_configure_start_thread()
{
    mp_tradingSimuThread = new TradingSimulationThread{};
    connect(this, &OrderBookWidget::finish_simulation, mp_tradingSimuThread, &TradingSimulationThread::finish_request);
    connect(mp_tradingSimuThread, &QThread::finished, mp_tradingSimuThread, &QObject::deleteLater);
    connect(mp_tradingSimuThread, &TradingSimulationThread::book_updated, this, &OrderBookWidget::update_book_presentation);
    mp_tradingSimuThread->start();
}

//=================================================

void OrderBookWidget::update_book_presentation(const PriceLevelsVector & askLvlsSnapshot,
                                               const PriceLevelsVector & bidLvlsSnapshot,
                                               double marketPrice,
                                               double spread)
{
    // Fill the asks table
    mp_asksTblWgt->setRowCount(askLvlsSnapshot.size());
    for (auto i = 0; i < askLvlsSnapshot.size(); ++i)
    {
        mp_asksTblWgt->setItem(askLvlsSnapshot.size() - 1 - i, 0, new QTableWidgetItem(QString::number(askLvlsSnapshot[i].first)));
        mp_asksTblWgt->setItem(askLvlsSnapshot.size() - 1 - i, 1, new QTableWidgetItem(QString::number(askLvlsSnapshot[i].second)));
    }
    mp_asksTblWgt->scrollToBottom(); // asks with the lowest prices have to be seen when book updates

    // Fill the bids table
    mp_bidsTblWgt->setRowCount(bidLvlsSnapshot.size());
    for (auto i = 0; i < bidLvlsSnapshot.size(); ++i)
    {
        mp_bidsTblWgt->setItem(i, 0, new QTableWidgetItem(QString::number(bidLvlsSnapshot[i].first)));
        mp_bidsTblWgt->setItem(i, 1, new QTableWidgetItem(QString::number(bidLvlsSnapshot[i].second)));
    }
    mp_bidsTblWgt->scrollToTop(); // bids with the highest prices have to be seen when book updates

    // Set the current market price
    if (marketPrice == -1.0) // if there weren't any transactions yet
        mp_marketPriceLbl->setText("Market price: ---");
    else
        mp_marketPriceLbl->setText("Market price: " + QString::number(marketPrice));

    // Set the current spread
    if (spread == -1.0) // if there aren't any orders of one of the types in the book
        mp_spreadLbl->setText("Spread: ---");
    else
        mp_spreadLbl->setText("Spread: " + QString::number(spread));

    this->repaint();
}
