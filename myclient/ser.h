
#pragma once

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "Order.h"
#include "Bars.h"
#include "Contract.h"
#include "Execution.h"
#include "Instrument.h"
#include "MTrade.h"

namespace boost {
namespace serialization {

template <class Archive>
void serialize(Archive &archive, TradeData &td, const unsigned int version)
{
    archive & td.numTrades;
    archive & td.tradeArr;
}

template <class Archive>
void serialize(Archive &archive, MTrade &t, const unsigned int version)
{
    /** read/write from/to archive **/
    archive & t.isOpen;
    archive & t.tradeId;
    archive & t.strategy;
    archive & t.instr_id;
    archive & t.openingOrder;
    archive & t.closingOrder;
    archive & t.openingExecution;
    archive & t.closingExecution;
}

/*

template <class Archive>
void serialize(Archive &archive, Instrument::RI &r, const unsigned int version)
{
    archive & r.orderContract;
    archive & r.dataContract;
    archive & r.realTimeBars;
    archive & r.historicalBars;
    archive & r.updatedBars;
}

template <class Archive>
void serialize(Archive &archive, Instrument &i, const unsigned int version)
{
    archive & i.inst_id;
    archive & i.barSize;
    archive & i.sec_barSize;
    archive & i.last_bar_update;
    archive & i.cross_validation_bars;
    archive & i.m_reqIds;
    archive & i.bars;
    archive & i.dataContract;
    archive & i.orderContract;
}

template <class Archive>
void serialize(Archive &archive, date_t &d, const unsigned int version)
{
    archive & d.tm_sec;
    archive & d.tm_min;
    archive & d.tm_hour;
    archive & d.tm_mday;
    archive & d.tm_mon;
    archive & d.tm_year;
    archive & d.tm_wday;
    archive & d.tm_yday;
    archive & d.tm_isdst;
}

*/

template <class Archive>
void serialize(Archive &archive, ComboLeg &cl, const unsigned int version)
{
    archive & cl.conId;
    archive & cl.ratio;
    archive & cl.action; 
    archive & cl.exchange;
    archive & cl.openClose; 
    archive & cl.shortSaleSlot; 
    archive & cl.designatedLocation;
    archive & cl.exemptCode;
}

template <class Archive>
void serialize(Archive &archive, DeltaNeutralContract &dn, const unsigned int version)
{
    archive & dn.conId;
    archive & dn.delta;
    archive & dn.price;
}

template <class Archive>
void serialize(Archive &archive, Contract &c, const unsigned int version)
{
    archive & c.conId;
    archive & c.symbol;
    archive & c.secType;
    archive & c.lastTradeDateOrContractMonth;
    archive & c.strike;
    archive & c.right;
    archive & c.multiplier;
    archive & c.exchange;
    archive & c.primaryExchange; 
    archive & c.localSymbol;
    archive & c.tradingClass;
    archive & c.includeExpired;
    archive & c.secIdType;		
    archive & c.secId;
    archive & c.description;
    archive & c.issuerId;
    archive & c.comboLegsDescrip;
    // archive & c.comboLegs;
    archive & c.deltaNeutralContract;
}

template <class Archive>
void serialize(Archive &archive, ContractDetails &cd, const unsigned int version)
{
    archive & cd.contract;
    archive & cd.marketName;
    archive & cd.minTick;
    archive & cd.orderTypes;
    archive & cd.validExchanges;
    archive & cd.priceMagnifier;
    archive & cd.underConId;
    archive & cd.longName;
    archive & cd.contractMonth;
    archive & cd.industry;
    archive & cd.category;
    archive & cd.subcategory;
    archive & cd.timeZoneId;
    archive & cd.tradingHours;
    archive & cd.liquidHours;
    archive & cd.evRule;
    archive & cd.evMultiplier;
    archive & cd.aggGroup;
    archive & cd.underSymbol;
    archive & cd.underSecType;
    archive & cd.marketRuleIds;
    archive & cd.realExpirationDate;
    archive & cd.lastTradeTime;
    archive & cd.stockType;
    archive & cd.minSize;
    archive & cd.sizeIncrement;
    archive & cd.suggestedSizeIncrement;
    // archive & cd.secIdList;
    archive & cd.cusip;
    archive & cd.ratings;
    archive & cd.descAppend;
    archive & cd.bondType;
    archive & cd.couponType;
    archive & cd.callable;
    archive & cd.putable;
    archive & cd.coupon;
    archive & cd.convertible;
    archive & cd.maturity;
    archive & cd.issueDate;
    archive & cd.nextOptionDate;
    archive & cd.nextOptionType;
    archive & cd.nextOptionPartial;
    archive & cd.notes;
}

template <class Archive>
void serialize(Archive &archive, OrderComboLeg &ol, const unsigned int version)
{
    archive & ol.price;
}

template <class Archive>
void serialize(Archive &archive, Order &o, const unsigned int version)
{ 
    archive & o.orderId;
    archive & o.clientId;
    archive & o.permId;
    archive & o.action;
    archive & o.totalQuantity;
    archive & o.orderType;
    archive & o.lmtPrice;
    archive & o.auxPrice;
    archive & o.tif;           
    archive & o.activeStartTime;	
    archive & o.activeStopTime;	
    archive & o.ocaGroup;     
    archive & o.ocaType;   
    archive & o.orderRef;    
    archive & o.transmit;      
    archive & o.parentId;     
    archive & o.blockOrder;
    archive & o.sweepToFill;
    archive & o.displaySize;
    archive & o.triggerMethod;
    archive & o.outsideRth;
    archive & o.hidden;
    archive & o.goodAfterTime;    
    archive & o.goodTillDate;    
    archive & o.rule80A; 
    archive & o.allOrNone;
    archive & o.minQty;
    archive & o.percentOffset; 
    archive & o.overridePercentageConstraints;
    archive & o.trailStopPrice;
    archive & o.trailingPercent;
    archive & o.faGroup;
    archive & o.faProfile;
    archive & o.faMethod;
    archive & o.faPercentage;
    archive & o.openClose;
    archive & o.origin;
    archive & o.shortSaleSlot;
    archive & o.designatedLocation; 
    archive & o.exemptCode;
    archive & o.discretionaryAmt;
    archive & o.optOutSmartRouting;
    archive & o.auctionStrategy;
    archive & o.startingPrice;
    archive & o.stockRefPrice;
    archive & o.delta;
    archive & o.stockRangeLower;
    archive & o.stockRangeUpper;
    archive & o.randomizeSize;
    archive & o.randomizePrice;
    archive & o.volatility;
    archive & o.volatilityType;
    archive & o.deltaNeutralOrderType;
    archive & o.deltaNeutralAuxPrice;
    archive & o.deltaNeutralConId;
    archive & o.deltaNeutralSettlingFirm;
    archive & o.deltaNeutralClearingAccount;
    archive & o.deltaNeutralClearingIntent;
    archive & o.deltaNeutralOpenClose;
    archive & o.deltaNeutralShortSale;
    archive & o.deltaNeutralShortSaleSlot;
    archive & o.deltaNeutralDesignatedLocation;
    archive & o.continuousUpdate;
    archive & o.referencePriceType;
    archive & o.basisPoints;
    archive & o.basisPointsType;
    archive & o.scaleInitLevelSize;
    archive & o.scaleSubsLevelSize;
    archive & o.scalePriceIncrement;
    archive & o.scalePriceAdjustValue;
    archive & o.scalePriceAdjustInterval;
    archive & o.scaleProfitOffset;
    archive & o.scaleAutoReset;
    archive & o.scaleInitPosition;
    archive & o.scaleInitFillQty;
    archive & o.scaleRandomPercent;
    archive & o.scaleTable;
    archive & o.hedgeType;  
    archive & o.hedgeParam; 
    archive & o.account; 
    archive & o.settlingFirm;
    archive & o.clearingAccount; 
    archive & o.clearingIntent;
    archive & o.algoStrategy;
    // archive & o.algoParams;
    // archive & o.smartComboRoutingParams;
    archive & o.algoId;
    archive & o.whatIf;
    archive & o.notHeld;
    archive & o.solicited;
    archive & o.modelCode;
    // archive & o.orderComboLegs;
    // archive & o.orderMiscOptions;
    archive & o.referenceContractId;
    archive & o.peggedChangeAmount;
    archive & o.isPeggedChangeAmountDecrease;
    archive & o.referenceChangeAmount;
    archive & o.referenceExchangeId;
    archive & o.adjustedOrderType;
    archive & o.triggerPrice;
    archive & o.adjustedStopPrice;
    archive & o.adjustedStopLimitPrice;
    archive & o.adjustedTrailingAmount;
    archive & o.adjustableTrailingUnit;
    archive & o.lmtPriceOffset;
    // archive & o.conditions;
    archive & o.conditionsCancelOrder;
    archive & o.conditionsIgnoreRth;
    archive & o.extOperator;
    // archive & o.softDollarTier;
    archive & o.cashQty;
    archive & o.mifid2DecisionMaker;
    archive & o.mifid2DecisionAlgo;
    archive & o.mifid2ExecutionTrader;
    archive & o.mifid2ExecutionAlgo;
    archive & o.dontUseAutoPriceForHedge;
    archive & o.isOmsContainer;
    archive & o.discretionaryUpToLimitPrice;
    archive & o.autoCancelDate;
    archive & o.filledQuantity;
    archive & o.refFuturesConId;
    archive & o.autoCancelParent;
    archive & o.shareholder;
    archive & o.imbalanceOnly;
    archive & o.routeMarketableToBbo;
    archive & o.parentPermId;
    archive & o.usePriceMgmtAlgo;
    archive & o.duration;
    archive & o.postToAts;
    archive & o.advancedErrorOverride;
    archive & o.manualOrderTime;
    archive & o.minTradeQty;
    archive & o.minCompeteSize;
    archive & o.competeAgainstBestOffset;
    archive & o.midOffsetAtWhole;
    archive & o.midOffsetAtHalf;
}

template <class Archive>
void serialize(Archive &archive, Execution &e, const unsigned int version)
{
    archive & e.execId;
    archive & e.time;
    archive & e.acctNumber;
    archive & e.exchange;
    archive & e.side;
    archive & e.shares;
    archive & e.price;
    archive & e.permId;
    archive & e.clientId;
    archive & e.orderId;
    archive & e.liquidation;
    archive & e.cumQty;
    archive & e.avgPrice;
    archive & e.orderRef;
    archive & e.evRule;
    archive & e.evMultiplier;
    archive & e.modelCode;
    archive & e.lastLiquidity;
}

}
}
