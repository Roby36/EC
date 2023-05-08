
#pragma once

#include "StdAfx.h"
#include "Utils.h"
#include "Decimal.h"
#include "Order.h"
#include "OrderState.h"
#include "CommonDefs.h"
#include "Contract.h"
#include "bar.h"
#include <string>

class Mlogger
{
  FILE* lfp;

  public:
  Mlogger(std::string);
  ~Mlogger();

  private:
  void printContractMsg(const Contract& contract);
  void printContractDetailsSecIdList(const TagValueListSPtr &secIdList);
  void printContractDetailsMsg(const ContractDetails& contractDetails);

  public:
  void str(std::string str);
  void currentTime( long time);
  void error(int id, int errorCode, const std::string& errorString, const std::string& advancedOrderRejectJson);
  void orderStatus(OrderId orderId, const std::string& status, Decimal filled,
  Decimal remaining, double avgFillPrice, int permId, int parentId,
  double lastFillPrice, int clientId, const std::string& whyHeld, double mktCapPrice);
  void openOrder( OrderId orderId, const Contract& contract, const Order& order, const OrderState& orderState);
  void contractDetails(int reqId, const ContractDetails& contractDetails);
  void historicalData(TickerId reqId, const Bar& bar);
  void realtimeBar(TickerId reqId, long time, double open, double high, double low, double close,
                        Decimal volume, Decimal wap, int count);
  void completedOrder(const Contract& contract, const Order& order, const OrderState& orderState);

  void printBondContractDetailsMsg(const ContractDetails& contractDetails);


};
