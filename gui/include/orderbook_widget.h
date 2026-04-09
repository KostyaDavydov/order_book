#pragma once

#include "orderbook.h"
#include "qcustomplot.h"

#include <QTableWidget>
#include <QLabel>
#include <QMainWindow>
#include <QWidget>
#include <QThread>

#include <atomic>

//=================================================
//=================================================

// The class describes a thread in which simulation process runs
class TradingSimulationThread : public QThread
{
    Q_OBJECT

private:
    // Order book
    OrderBook m_book;

    // Flag that is used to finish simulation
    std::atomic<bool> m_finish {false};

    // Main thread function (simulation process)
    void run() override;

public slots:
    // Request to finish simulation
    void finish_request();

signals:
    // Signal provides all necessary information to update
    // the order book presentation in the main window
    void book_updated(const PriceLevelsVector & askLvlsSnapshot,
                      const PriceLevelsVector & bidLvlsSnapshot,
                      double marketPrice,
                      double spread);
};

//=================================================

class OrderBookWidget : public QMainWindow
{
    Q_OBJECT

private:
// Widgets
    QTableWidget * mp_asksTblWgt;
    QTableWidget * mp_bidsTblWgt;
    QLabel * mp_marketPriceLbl;
    QLabel * mp_spreadLbl;

    QCustomPlot * mp_DOMCustomPlot;

// Menu actions
    QAction * mp_startSimuAct;
    QAction * mp_stopSimuAct;

// Thread for trading simulation process
    TradingSimulationThread * mp_tradingSimuThread;

    // Create, configure, and start the trading simulation thread
    void create_configure_start_thread();

public:
    explicit OrderBookWidget(QWidget * parent = nullptr);
    ~OrderBookWidget();

signals:
    void finish_simulation();

public slots:
    // Update the order book presentation in the main window
    void update_book_presentation(const PriceLevelsVector & askLvlsSnapshot,
                                  const PriceLevelsVector & bidLvlsSnapshot,
                                  double marketPrice,
                                  double spread);
};
