
#include "MOrders.h"

Order MOrders::MarketOrder(std::string action, Decimal quantity, std::string orderRef) {
	Order order;
	order.action = action;
	order.orderType = "MKT";
	order.totalQuantity = quantity;
	order.orderRef = orderRef;
	return order;
}

Order MOrders::MarketOnClose(std::string action, Decimal quantity, std::string orderRef) {
	Order order;
	order.action = action;
	order.orderType = "MOC";
	order.totalQuantity = quantity;
	order.orderRef = orderRef;
	return order;
}

Order MOrders::MarketOnOpen(std::string action, Decimal quantity, std::string orderRef) {
	Order order;
	order.action = action;
	order.orderType = "MKT";
	order.totalQuantity = quantity;
	order.tif = "OPG";
	order.orderRef = orderRef;
	return order;
}

Order MOrders::LimitOrder(std::string action, Decimal quantity, double limitPrice, std::string orderRef) {
	Order order;
	order.action = action;
	order.orderType = "LMT";
	order.totalQuantity = quantity;
	order.lmtPrice = limitPrice;
	order.orderRef = orderRef;
	return order;
}

Order MOrders::MarketToLimit(std::string action, Decimal quantity, std::string orderRef) {
	Order order;
	order.action = action;
	order.orderType = "MTL";
	order.totalQuantity = quantity;
	order.orderRef = orderRef;
	return order;
}

Order MOrders::MarketWithProtection(std::string action, Decimal quantity, std::string orderRef) {
	Order order;
	order.action = action;
	order.orderType = "MKT PRT";
	order.totalQuantity = quantity;
	order.orderRef = orderRef;
	return order;
}

Order MOrders::Stop(std::string action, Decimal quantity, double stopPrice, std::string orderRef) {
	Order order;
	order.action = action;
	order.orderType = "STP";
	order.totalQuantity = quantity;
	order.auxPrice = stopPrice;
	order.orderRef = orderRef;
	return order;
}

Order MOrders::StopLimit(std::string action, Decimal quantity, double limitPrice, double stopPrice, std::string orderRef) {
	Order order;
	order.action = action;
	order.orderType = "STP LMT";
	order.totalQuantity = quantity;
	order.lmtPrice = limitPrice;
	order.auxPrice = stopPrice;
	order.orderRef = orderRef;
	return order;
}

Order MOrders::StopWithProtection(std::string action, Decimal quantity, double stopPrice, std::string orderRef) {
	Order order;
	order.action = action;
	order.orderType = "STP PRT";
	order.totalQuantity = quantity;
	order.auxPrice = stopPrice;
	order.orderRef = orderRef;
	return order;
}

Order MOrders::ComboMarketOrder(std::string action, Decimal quantity, bool nonGuaranteed, std::string orderRef) {
	Order order;
	order.action = action;
	order.orderType = "MKT";
	order.totalQuantity = quantity;
	if(nonGuaranteed){
		TagValueSPtr tag1(new TagValue("NonGuaranteed", "1"));
		order.smartComboRoutingParams.reset(new TagValueList());
		order.smartComboRoutingParams->push_back(tag1);
	}
	order.orderRef = orderRef;
	return order;
}

void MOrders::OneCancelsAll(std::string ocaGroup, Order& ocaOrder, int ocaType, std::string orderRef){
	ocaOrder.ocaGroup = ocaGroup;
	ocaOrder.ocaType = ocaType;
	ocaOrder.orderRef = orderRef;
}

