SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for account
-- ----------------------------
DROP TABLE IF EXISTS `account`;
CREATE TABLE `account` (
  `investor_id` char(16) NOT NULL DEFAULT '',
  `password` varchar(50) NOT NULL,
  `broker_id` char(5) NOT NULL,
  `front_address` varchar(100) NOT NULL,
  PRIMARY KEY (`investor_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for account_position
-- ----------------------------
DROP TABLE IF EXISTS `account_position`;
CREATE TABLE `account_position` (
  `investor_id` char(16) NOT NULL DEFAULT '',
  `instrument_id` char(6) NOT NULL DEFAULT '',
  `long_position` int(11) NOT NULL DEFAULT '0',
  `long_profit` double NOT NULL DEFAULT '0',
  `long_margin` double NOT NULL DEFAULT '0',
  `short_position` int(11) NOT NULL DEFAULT '0',
  `short_profit` double NOT NULL DEFAULT '0',
  `short_margin` double NOT NULL DEFAULT '0',
  `query_date` date NOT NULL,
  PRIMARY KEY (`investor_id`,`instrument_id`),
  CONSTRAINT `account_position_ibfk_1` FOREIGN KEY (`investor_id`) REFERENCES `account` (`investor_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for account_strategy
-- ----------------------------
DROP TABLE IF EXISTS `account_strategy`;
CREATE TABLE `account_strategy` (
  `investor_id` char(16) DEFAULT NULL,
  `strategy_id` char(10) DEFAULT NULL,
  `available` double DEFAULT NULL,
  KEY `investor_id` (`investor_id`),
  KEY `strategy_id` (`strategy_id`),
  CONSTRAINT `account_strategy_ibfk_2` FOREIGN KEY (`strategy_id`) REFERENCES `strategy` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `account_strategy_ibfk_1` FOREIGN KEY (`investor_id`) REFERENCES `account` (`investor_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for close_profit
-- ----------------------------
DROP TABLE IF EXISTS `close_profit`;
CREATE TABLE `close_profit` (
  `close_date` date NOT NULL DEFAULT '0000-00-00',
  `close_id` char(21) NOT NULL DEFAULT '',
  `open_date` date NOT NULL DEFAULT '0000-00-00',
  `open_id` char(21) NOT NULL DEFAULT '',
  `close_price` double DEFAULT NULL,
  `open_price` double DEFAULT NULL,
  `volume` int(11) DEFAULT NULL,
  `profit` double DEFAULT NULL,
  PRIMARY KEY (`close_date`,`close_id`,`open_date`,`open_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for close_traded_report
-- ----------------------------
DROP TABLE IF EXISTS `close_traded_report`;
CREATE TABLE `close_traded_report` (
  `trade_date` date NOT NULL DEFAULT '0000-00-00',
  `trade_id` char(21) NOT NULL DEFAULT '',
  `trade_time` time DEFAULT NULL,
  `investor_id` char(16) DEFAULT NULL,
  `strategy_id` char(10) DEFAULT NULL,
  `system_id` char(21) DEFAULT NULL,
  `order_ref` int(11) DEFAULT NULL,
  `instrument_id` char(6) DEFAULT NULL,
  `direction` char(1) DEFAULT NULL,
  `volume` int(11) DEFAULT NULL,
  `close_price` double DEFAULT NULL,
  `today_flag` char(1) DEFAULT NULL,
  PRIMARY KEY (`trade_date`,`trade_id`),
  KEY `investor_id` (`investor_id`),
  KEY `strategy_id` (`strategy_id`),
  CONSTRAINT `close_traded_report_ibfk_2` FOREIGN KEY (`strategy_id`) REFERENCES `strategy` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `close_traded_report_ibfk_1` FOREIGN KEY (`investor_id`) REFERENCES `account` (`investor_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for fund
-- ----------------------------
DROP TABLE IF EXISTS `fund`;
CREATE TABLE `fund` (
  `investor_id` char(16) NOT NULL DEFAULT '',
  `pre_balance` double DEFAULT NULL,
  `deposit` double DEFAULT NULL,
  `withdraw` double DEFAULT NULL,
  `available` double DEFAULT NULL,
  `current_margin` double DEFAULT NULL,
  `frozen_margin` double DEFAULT NULL,
  `commission` double DEFAULT NULL,
  `close_profit` double DEFAULT NULL,
  `position_profit` double DEFAULT NULL,
  PRIMARY KEY (`investor_id`),
  CONSTRAINT `fund_ibfk_1` FOREIGN KEY (`investor_id`) REFERENCES `account` (`investor_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for history_fund
-- ----------------------------
DROP TABLE IF EXISTS `history_fund`;
CREATE TABLE `history_fund` (
  `investor_id` char(16) NOT NULL DEFAULT '',
  `record_time` date DEFAULT NULL,
  `pre_balance` double DEFAULT NULL,
  `deposit` double DEFAULT NULL,
  `withdraw` double DEFAULT NULL,
  `available` double DEFAULT NULL,
  `current_margin` double DEFAULT NULL,
  `frozen_margin` double DEFAULT NULL,
  `commission` double DEFAULT NULL,
  `close_profit` double DEFAULT NULL,
  `position_profit` double DEFAULT NULL,
  PRIMARY KEY (`investor_id`),
  CONSTRAINT `history_fund_ibfk_1` FOREIGN KEY (`investor_id`) REFERENCES `account` (`investor_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for instrument_info
-- ----------------------------
DROP TABLE IF EXISTS `instrument_info`;
CREATE TABLE `instrument_info` (
  `id` char(6) NOT NULL DEFAULT '',
  `name` varchar(20) DEFAULT NULL,
  `exchange_id` char(4) DEFAULT NULL,
  `deadline` date DEFAULT NULL,
  `margin_rate` double DEFAULT NULL,
  `multiplier` int(11) DEFAULT NULL,
  `minimum_unit` double DEFAULT NULL,
  `oc` double NOT NULL DEFAULT '0',
  `oc_rate` double NOT NULL DEFAULT '0',
  `cc` double NOT NULL DEFAULT '0',
  `cc_rate` double NOT NULL DEFAULT '0',
  `today_cc` double NOT NULL DEFAULT '0',
  `today_cc_rate` double NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for open_traded_report
-- ----------------------------
DROP TABLE IF EXISTS `open_traded_report`;
CREATE TABLE `open_traded_report` (
  `trade_date` date NOT NULL DEFAULT '0000-00-00',
  `trade_id` char(21) NOT NULL DEFAULT '',
  `trade_time` time DEFAULT NULL,
  `investor_id` char(16) DEFAULT NULL,
  `strategy_id` char(10) DEFAULT NULL,
  `system_id` char(21) DEFAULT NULL,
  `order_ref` int(11) DEFAULT NULL,
  `instrument_id` char(6) DEFAULT NULL,
  `direction` char(1) DEFAULT NULL,
  `volume` int(11) DEFAULT NULL,
  `open_price` double DEFAULT NULL,
  `to_be_close` int(11) DEFAULT NULL,
  PRIMARY KEY (`trade_date`,`trade_id`),
  KEY `investor_id` (`investor_id`),
  KEY `strategy_id` (`strategy_id`),
  CONSTRAINT `open_traded_report_ibfk_2` FOREIGN KEY (`strategy_id`) REFERENCES `strategy` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `open_traded_report_ibfk_1` FOREIGN KEY (`investor_id`) REFERENCES `account` (`investor_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for orders
-- ----------------------------
DROP TABLE IF EXISTS `orders`;
CREATE TABLE `orders` (
  `investor_id` char(16) NOT NULL DEFAULT '',
  `order_date` date NOT NULL DEFAULT '0000-00-00',
  `order_ref` int(11) NOT NULL DEFAULT '0',
  `strategy_id` char(10) NOT NULL DEFAULT '',
  `instruction_id` char(15) NOT NULL DEFAULT '',
  `system_id` char(21) DEFAULT NULL,
  `sequence_num` int(11) DEFAULT NULL,
  `instrument_id` char(6) DEFAULT NULL,
  `direction` char(1) DEFAULT NULL,
  `close_open_flag` char(1) DEFAULT NULL,
  `price` double DEFAULT NULL,
  `original_volume` int(11) DEFAULT NULL,
  `traded_volume` int(11) DEFAULT NULL,
  `rest_volume` int(11) DEFAULT NULL,
  `order_status` char(1) DEFAULT NULL,
  PRIMARY KEY (`investor_id`,`order_date`,`order_ref`),
  KEY `strategy_id` (`strategy_id`),
  CONSTRAINT `orders_ibfk_2` FOREIGN KEY (`strategy_id`) REFERENCES `strategy` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `orders_ibfk_1` FOREIGN KEY (`investor_id`) REFERENCES `account` (`investor_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for strategy
-- ----------------------------
DROP TABLE IF EXISTS `strategy`;
CREATE TABLE `strategy` (
  `id` char(10) NOT NULL DEFAULT '',
  `name` varchar(50) DEFAULT NULL,
  `interested_instruments` varchar(200) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for strategy_position
-- ----------------------------
DROP TABLE IF EXISTS `strategy_position`;
CREATE TABLE `strategy_position` (
  `investor_id` char(16) NOT NULL DEFAULT '',
  `strategy_id` char(10) NOT NULL DEFAULT '',
  `instrument_id` char(6) NOT NULL DEFAULT '',
  `long_position` int(11) NOT NULL DEFAULT '0',
  `short_position` int(11) NOT NULL DEFAULT '0',
  `today_long_position` int(11) NOT NULL DEFAULT '0',
  `today_short_position` int(11) NOT NULL DEFAULT '0',
  `today` date DEFAULT NULL,
  PRIMARY KEY (`investor_id`,`strategy_id`,`instrument_id`),
  KEY `strategy_id` (`strategy_id`),
  CONSTRAINT `strategy_position_ibfk_2` FOREIGN KEY (`strategy_id`) REFERENCES `strategy` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `strategy_position_ibfk_1` FOREIGN KEY (`investor_id`) REFERENCES `account` (`investor_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
