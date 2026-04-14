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
    // Each action as well as parameters for actions are generated randomly
    std::random_device rd;
    std::mt19937 gen(rd());

    std::poisson_distribution<> actionDist(4);

    // ASK - 0; BID - 1
    std::uniform_int_distribution<> typeDist(0, 1);

    // Order price and volume
    std::normal_distribution<> priceDist(50.0, 7.0);
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
            auto action = actionDist(gen);

            if (action <= 4)
            {
                m_book.add_order(currId++,
                                 priceDist(gen),
                                 volumeDist(gen),
                                 static_cast<OrderType>(typeDist(gen)));
            }
            else if (action == 5)
            {
                if (currId == 0) // no orders were added yet
                    continue;
                std::uniform_int_distribution<> idDist(0, currId - 1);
                m_book.delete_order(idDist(gen));
            }
            else if (action == 6)
            {
                if (currId == 0) // no orders were added yet
                    continue;
                std::uniform_int_distribution<> idDist(0, currId - 1);
                m_book.modify_order(idDist(gen), priceDist(gen), volumeDist(gen));
            }
            else
            {
                m_book.execute(static_cast<OrderType>(typeDist(gen)), volumeDist(gen));
            }
        }

        // Notify the main thread that the book was updated
        emit book_updated(m_book.price_levels_for_type(OrderType::ASK),
                          m_book.price_levels_for_type(OrderType::BID),
                          m_book.market_price(),
                          m_book.spread());

        // Little delay before the next action
        QThread::msleep(500);
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

    init_tables();

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

    init_custom_plot();

    // Set the splitter between the book and the DOM

    auto mainSplitter = new QSplitter;
    mainSplitter->addWidget(mp_DOMCustomPlot);
    mainSplitter->addWidget(bookPartWgt);

    // Register the complex type so that objects of that type are able
    // to be transferred using the signal/slot mechanism
    qRegisterMetaType<PriceLevelsVector>();

    // Create and configure a menu bar

    init_menu();

    // Last display settings

    setCentralWidget(mainSplitter);
    setMinimumSize(300, 300);
    resize(1000, 500);
    mainSplitter->setSizes({500, 500});
    apply_styles();
}

//=================================================

OrderBookWidget::~OrderBookWidget()
{
    // Stop the simulation thread and wait until it completely terminates
    emit finish_simulation();
    mp_tradingSimuThread->wait();
}

//=================================================

void OrderBookWidget::init_tables()
{
    mp_asksTblWgt = new QTableWidget;
    mp_asksTblWgt->setColumnCount(2);
    mp_asksTblWgt->setHorizontalHeaderLabels({"Price", "Volume"});
    mp_asksTblWgt->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mp_asksTblWgt->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    mp_asksTblWgt->setStyleSheet("QTableWidget::item {background-color: rgba(255, 0, 0, 100);}");
    mp_asksTblWgt->setEditTriggers(QAbstractItemView::NoEditTriggers);

    mp_bidsTblWgt = new QTableWidget;
    mp_bidsTblWgt->setColumnCount(2);
    mp_bidsTblWgt->setHorizontalHeaderLabels({"Price", "Volume"});
    mp_bidsTblWgt->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mp_bidsTblWgt->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    mp_bidsTblWgt->setStyleSheet("QTableWidget::item {background-color: rgba(0, 255, 0, 100);}");
    mp_bidsTblWgt->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

//=================================================

void OrderBookWidget::init_custom_plot()
{
    mp_DOMCustomPlot = new QCustomPlot;

    // Configure axes
    mp_DOMCustomPlot->xAxis->setLabel("Price");
    mp_DOMCustomPlot->yAxis->setLabel("Cumulative Volume");

    // Add and configure graphs
    mp_DOMCustomPlot->addGraph(); // Graph 0 (for bids)
    mp_DOMCustomPlot->graph(0)->setPen(QPen(Qt::green, 2)); // Green line, thickness 2
    mp_DOMCustomPlot->graph(0)->setBrush(QBrush(QColor(0, 255, 0, 100))); // Green fill with transparency
    mp_DOMCustomPlot->graph(0)->setName("BIDS");

    mp_DOMCustomPlot->addGraph(); // Graph 1 (for asks)
    mp_DOMCustomPlot->graph(1)->setPen(QPen(Qt::red, 2)); // Red line, thickness 2
    mp_DOMCustomPlot->graph(1)->setBrush(QBrush(QColor(255, 0, 0, 100))); // Red fill with transparency
    mp_DOMCustomPlot->graph(1)->setName("ASKS");

    mp_DOMCustomPlot->setBackground(QBrush(QColor("#ecf0f1")));

    // Add legend to distinguish the graphs
    mp_DOMCustomPlot->legend->setVisible(true);
    mp_DOMCustomPlot->legend->setBrush(QBrush(QColor(255, 255, 255, 200))); // semi-transparent background
}

//=================================================

void OrderBookWidget::init_menu()
{
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

void OrderBookWidget::apply_styles()
{
    QString styleSheet =
        // Main window
        "QMainWindow { background-color: #2c3e50; }"
        "QWidget#centralWidget { background-color: #ecf0f1; }"

        // Menu
        "QMenuBar { background-color: #34495e; color: #ecf0f1; border-bottom: 1px solid #2c3e50; }"
        "QMenuBar::item:selected { background-color: #2c3e50; }"
        "QMenu { background-color: #34495e; color: #ecf0f1; border: 1px solid #2c3e50; }"
        "QMenu::item:selected { background-color: #1abc9c; }"

        // Labels
        "QLabel { color: white; }"

        // Tables
        "QTableWidget { background-color: #ecf0f1; alternate-background-color: #f8f9fa; gridline-color: #dee2e6; selection-background-color: rgba(26, 188, 156, 150); selection-color: white; }"
        "QHeaderView::section { background-color: #34495e; color: white; padding: 8px; border: 1px solid #2c3e50; font-weight: bold; }"

        // Scrollbars
        "QScrollBar:vertical { background-color: #ecf0f1; width: 12px; border-radius: 6px; }"
        "QScrollBar::handle:vertical { background-color: #bdc3c7; border-radius: 6px; min-height: 20px; }"
        "QScrollBar::handle:vertical:hover { background-color: #95a5a6; }";

    this->setStyleSheet(styleSheet);
}

//=================================================

void OrderBookWidget::update_book_presentation(const PriceLevelsVector & askLvlsSnapshot,
                                               const PriceLevelsVector & bidLvlsSnapshot,
                                               double marketPrice,
                                               double spread)
{
    int askLvlsCnt = askLvlsSnapshot.size();
    int bidLvlsCnt = bidLvlsSnapshot.size();

    // Set the appropriate sizes
    m_askPrices.resize(askLvlsCnt);
    m_bidPrices.resize(bidLvlsCnt);
    m_askCumulVolumes.resize(askLvlsCnt);
    m_bidCumulVolumes.resize(bidLvlsCnt);

    // Fill the asks data
    mp_asksTblWgt->setRowCount(askLvlsCnt);
    for (auto i = 0; i < askLvlsCnt; ++i)
    {
        // Table
        mp_asksTblWgt->setItem(askLvlsCnt - i - 1, 0, new QTableWidgetItem(QString::number(askLvlsSnapshot[i].first)));
        mp_asksTblWgt->setItem(askLvlsCnt - i - 1, 1, new QTableWidgetItem(QString::number(askLvlsSnapshot[i].second)));

        // Graph

        // X axis
        m_askPrices[i] = askLvlsSnapshot[i].first;

        // Y axis
        if (i != 0)
            m_askCumulVolumes[i] = m_askCumulVolumes[i - 1] + askLvlsSnapshot[i].second;
        else
            m_askCumulVolumes[i] = askLvlsSnapshot[i].second;
    }
    mp_asksTblWgt->scrollToBottom(); // asks with the lowest prices have to be seen when book updates

    // Fill the bids table
    mp_bidsTblWgt->setRowCount(bidLvlsCnt);
    for (auto i = 0; i < bidLvlsCnt; ++i)
    {
        // Table
        mp_bidsTblWgt->setItem(i, 0, new QTableWidgetItem(QString::number(bidLvlsSnapshot[i].first)));
        mp_bidsTblWgt->setItem(i, 1, new QTableWidgetItem(QString::number(bidLvlsSnapshot[i].second)));

        // Graph

        // X axis
        m_bidPrices[i] = bidLvlsSnapshot[bidLvlsCnt - i - 1].first;

        // Y axis
        if (i != 0)
            m_bidCumulVolumes[bidLvlsCnt - i - 1] = m_bidCumulVolumes[bidLvlsCnt - i] + bidLvlsSnapshot[i].second;
        else
            m_bidCumulVolumes[bidLvlsCnt - i - 1] = bidLvlsSnapshot[i].second;
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

    update_graphs(askLvlsCnt, bidLvlsCnt);
    this->repaint();
}

//=================================================

void OrderBookWidget::update_graphs(int askLvlsCnt, int bidLvlsCnt)
{
    mp_DOMCustomPlot->graph(0)->setData(m_bidPrices, m_bidCumulVolumes, true); // green graph (BIDS)
    mp_DOMCustomPlot->graph(1)->setData(m_askPrices, m_askCumulVolumes, true); // red graph (ASKS

    if (askLvlsCnt != 0 && bidLvlsCnt != 0) // there are asks and bids in the book
    {
        mp_DOMCustomPlot->xAxis->setRange(m_bidPrices.front(), m_askPrices.back());
        mp_DOMCustomPlot->yAxis->setRange(0, std::max(m_bidCumulVolumes.front(), m_askCumulVolumes.back()));
    }
    else if (askLvlsCnt != 0) // there are only asks in the book
    {
        mp_DOMCustomPlot->xAxis->setRange(m_askPrices.front(), m_askPrices.back());
        mp_DOMCustomPlot->yAxis->setRange(0, m_askCumulVolumes.back());
    }
    else if (bidLvlsCnt != 0) // there are only bids in the book
    {
        mp_DOMCustomPlot->xAxis->setRange(m_bidPrices.front(), m_bidPrices.back());
        mp_DOMCustomPlot->yAxis->setRange(0, m_bidCumulVolumes.front());
    }

    // Refresh the plot
    mp_DOMCustomPlot->replot();
}
