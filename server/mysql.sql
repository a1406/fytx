/*
SQLyog Trial v9.33 GA
MySQL - 5.1.61-log : Database - sanguo
*********************************************************************
*/


CREATE DATABASE /*!32312 IF NOT EXISTS*/`pay` /*!40100 DEFAULT CHARACTER SET utf8 COLLATE utf8_unicode_ci */;
USE `pay`;
CREATE TABLE `pay_log` (
  `player_id` int(11) NOT NULL AUTO_INCREMENT,
  `pay_time` datetime NOT NULL,
  `pay_time` datetime NOT NULL,
  `server_id` int(11) NOT NULL,
  `receipt_data` varchar(32) DEFAULT NULL,
  KEY (`player_id`),
  KEY (`receipt_data`)
) ENGINE=MyISAM AUTO_INCREMENT=8 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE `err_log` (
  `player_id` int(11) NOT NULL AUTO_INCREMENT,
  `pay_time` datetime NOT NULL,
  `server_id` int(11) NOT NULL,
  `receipt_data` varchar(32) DEFAULT NULL,
  `result` varchar(32) DEFAULT NULL,
  KEY (`player_id`),
  KEY (`receipt_data`)
) ENGINE=MyISAM AUTO_INCREMENT=8 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE `share` (
  `server_id` int(11) NOT NULL,
  `first_share_time` datetime NOT NULL,
  `last_share_time` datetime NOT NULL,
  `share_times` int(2) NOT NULL,
  `pay_times` int(4) NOT NULL,
  `player_id` int(11) NOT NULL,
  PRIMARY KEY (`player_id`, `server_id`)
) ENGINE=MyISAM AUTO_INCREMENT=8 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

/*!40101 SET NAMES utf8 */;

/*!40101 SET SQL_MODE=''*/;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
CREATE DATABASE /*!32312 IF NOT EXISTS*/`sanguo` /*!40100 DEFAULT CHARACTER SET utf8 COLLATE utf8_unicode_ci */;

USE `sanguo`;

/*Table structure for table `admins` */

DROP TABLE IF EXISTS `admins`;

CREATE TABLE `admins` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `role_id` int(11) NOT NULL,
  `username` varchar(50) COLLATE utf8_unicode_ci NOT NULL,
  `password` varchar(32) COLLATE utf8_unicode_ci NOT NULL,
  `last_time` datetime NOT NULL,
  `lock_time` datetime DEFAULT NULL,
  `status` int(11) NOT NULL,
  `login_count` int(11) DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `admins_bf07f040` (`role_id`)
) ENGINE=MyISAM AUTO_INCREMENT=8 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

/*Data for the table `admins` */

insert  into `admins`(`id`,`role_id`,`username`,`password`,`last_time`,`lock_time`,`status`,`login_count`) values (1,1,'root','b8a7bfd173d28b5367cc52588f85cc04','2013-07-02 21:27:53','2012-10-17 21:01:58',0,0),(2,3,'fengyuntianxia01','c172e5f7f745740338a48dd23848118e','2012-04-27 16:04:40',NULL,0,NULL),(3,3,'fengyuntianxia02','c172e5f7f745740338a48dd23848118e','2012-04-27 16:04:56',NULL,0,NULL),(4,8,'abc','76539b8724f9ce350018aa9b1aa2030a','2013-02-14 20:58:08',NULL,0,0),(5,4,'cba','6354d46c5e4f56b3a3aa78530fca9692','2013-02-14 20:55:43',NULL,0,0),(6,5,'xv','0b18c477f7c888f9029fa654475c3b05','2012-11-17 12:55:20',NULL,5,0),(7,10,'ledou','13e839a6afb79087a9d37c562e39f627','2013-06-07 23:31:56',NULL,0,0);

DROP TABLE IF EXISTS `account`;

CREATE TABLE `account` (
  `open_id` int(11) NOT NULL AUTO_INCREMENT,
  `username` varchar(50) COLLATE utf8_unicode_ci NOT NULL,
  `password` varchar(128) COLLATE utf8_unicode_ci NOT NULL,
  `create_time` datetime NOT NULL,
  `login_time` datetime DEFAULT NULL,
  `status` int(11) NOT NULL,
  `login_count` int(11) DEFAULT '0',
  `key` varchar(32),
  PRIMARY KEY (`open_id`),
  KEY `account_bf07f040` (`username`),
  KEY  (`password`),
  UNIQUE (`username`)
) ENGINE=MyISAM AUTO_INCREMENT=8 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE `game_account_15` (
  `open_id` int(32) NOT NULL,
  `user_type` int(8) NOT NULL,	
  `player_id` int(32) NOT NULL  AUTO_INCREMENT,	
  `username` varchar(50) COLLATE utf8_unicode_ci,
  `create_time` datetime,
  `login_time` datetime,
  `status` int(11),
  `login_count` int(11) DEFAULT '0',
  PRIMARY KEY (`player_id`),
  KEY (`open_id`, `user_type`),
  UNIQUE (`open_id`, `user_type`)
) ENGINE=MyISAM AUTO_INCREMENT=8 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
insert into `game_account` (`open_id`, `user_type`, `player_id`) values(0,0,0);

CREATE TABLE `question` (
  `q_id` int(11) NOT NULL   AUTO_INCREMENT,
  `player_id` int(11) NOT NULL,
  `server_id` int(11) NOT NULL,
  `question_content` varchar(150) COLLATE utf8_unicode_ci NOT NULL,
  `question_time` datetime DEFAULT NULL,
  `question_type` int(11) NOT NULL,
  `question_state` int(4) NOT NULL,
  `answer_content` varchar(150) COLLATE utf8_unicode_ci,
  `answer_time` datetime DEFAULT NULL,
  `answer_name` varchar(32),
   PRIMARY KEY (`q_id`)
) ENGINE=MyISAM AUTO_INCREMENT=8 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE `player_15` (
  `player_id` int(11) NOT NULL AUTO_INCREMENT,
  `player_name` varchar(50) COLLATE utf8_unicode_ci NOT NULL,
  `channel_id` int(11) NOT NULL,
  `user_type` int(11) NOT NULL,
  `link_key` varchar(32) COLLATE utf8_unicode_ci NOT NULL,
  `status` int(11) NOT NULL,
  `login_num` int(11) DEFAULT '0',
  `last_time` datetime DEFAULT NULL,
   PRIMARY KEY (`player_id`)
) ENGINE=MyISAM AUTO_INCREMENT=8 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE `log_login` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `log_type` int(11) NOT NULL,
  `log_user` int(11) NOT NULL,
  `log_server` int(11) NOT NULL,
  `log_channel` int(11) NOT NULL,
  `log_data` int(11) NOT NULL,
  `log_result` int(11) NOT NULL,
  `log_time` datetime NOT NULL,
  `f1` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f2` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f3` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f4` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f5` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f6` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE `log_register` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `log_type` int(11) NOT NULL,
  `log_user` int(11) NOT NULL,
  `log_server` int(11) NOT NULL,
  `log_channel` int(11) NOT NULL,
  `log_data` int(11) NOT NULL,
  `log_result` int(11) NOT NULL,
  `log_time` datetime NOT NULL,
  `f1` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f2` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f3` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f4` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f5` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f6` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE `log_steps` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `log_type` int(11) NOT NULL,
  `log_user` int(11) NOT NULL,
  `log_server` int(11) NOT NULL,
  `log_channel` int(11) NOT NULL,
  `log_data` int(11) NOT NULL,
  `log_result` int(11) NOT NULL,
  `log_time` datetime NOT NULL,
  `f1` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f2` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f3` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f4` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f5` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f6` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE `log_playing` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `log_type` int(11) NOT NULL,
  `log_user` int(11) NOT NULL,
  `log_server` int(11) NOT NULL,
  `log_channel` int(11) NOT NULL,
  `log_data` int(11) NOT NULL,
  `log_result` int(11) NOT NULL,
  `log_time` datetime NOT NULL,
  `f1` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f2` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f3` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f4` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f5` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f6` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE `log_gold` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `log_type` int(11) NOT NULL,
  `log_user` int(11) NOT NULL,
  `log_server` int(11) NOT NULL,
  `log_channel` int(11) NOT NULL,
  `log_data` int(11) NOT NULL,
  `log_result` int(11) NOT NULL,
  `log_time` datetime NOT NULL,
  `f1` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f2` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f3` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f4` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f5` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f6` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE `log_create_role` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `log_type` int(11) NOT NULL,
  `log_user` int(11) NOT NULL,
  `log_server` int(11) NOT NULL,
  `log_channel` int(11) NOT NULL,
  `log_data` int(11) NOT NULL,
  `log_result` int(11) NOT NULL,
  `log_time` datetime NOT NULL,
  `f1` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f2` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f3` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f4` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f5` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f6` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE `log_equipment` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `log_type` int(11) NOT NULL,
  `log_user` int(11) NOT NULL,
  `log_server` int(11) NOT NULL,
  `log_channel` int(11) NOT NULL,
  `log_data` int(11) NOT NULL,
  `log_result` int(11) NOT NULL,
  `log_time` datetime NOT NULL,
  `f1` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f2` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f3` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f4` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f5` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f6` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE `log_online` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `log_type` int(11) NOT NULL,
  `log_user` int(11) NOT NULL,
  `log_server` int(11) NOT NULL,
  `log_channel` int(11) NOT NULL,
  `log_data` int(11) NOT NULL,
  `log_result` int(11) NOT NULL,
  `log_time` datetime NOT NULL,
  `f1` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f2` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f3` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f4` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f5` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f6` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE `log_stage` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `log_type` int(11) NOT NULL,
  `log_user` int(11) NOT NULL,
  `log_server` int(11) NOT NULL,
  `log_channel` int(11) NOT NULL,
  `log_data` int(11) NOT NULL,
  `log_result` int(11) NOT NULL,
  `log_time` datetime NOT NULL,
  `f1` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f2` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f3` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f4` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f5` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f6` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE `log_silver` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `log_type` int(11) NOT NULL,
  `log_user` int(11) NOT NULL,
  `log_server` int(11) NOT NULL,
  `log_channel` int(11) NOT NULL,
  `log_data` int(11) NOT NULL,
  `log_result` int(11) NOT NULL,
  `log_time` datetime NOT NULL,
  `f1` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f2` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f3` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f4` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f5` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f6` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE `log_level` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `log_type` int(11) NOT NULL,
  `log_user` int(11) NOT NULL,
  `log_server` int(11) NOT NULL,
  `log_channel` int(11) NOT NULL,
  `log_data` int(11) NOT NULL,
  `log_result` int(11) NOT NULL,
  `log_time` datetime NOT NULL,
  `f1` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f2` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f3` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f4` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f5` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f6` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE `log_jungong` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `log_type` int(11) NOT NULL,
  `log_user` int(11) NOT NULL,
  `log_server` int(11) NOT NULL,
  `log_channel` int(11) NOT NULL,
  `log_data` int(11) NOT NULL,
  `log_result` int(11) NOT NULL,
  `log_time` datetime NOT NULL,
  `f1` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f2` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f3` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f4` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f5` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f6` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE `log_junling` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `log_type` int(11) NOT NULL,
  `log_user` int(11) NOT NULL,
  `log_server` int(11) NOT NULL,
  `log_channel` int(11) NOT NULL,
  `log_data` int(11) NOT NULL,
  `log_result` int(11) NOT NULL,
  `log_time` datetime NOT NULL,
  `f1` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f2` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f3` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f4` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f5` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f6` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE `log_weiwang` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `log_type` int(11) NOT NULL,
  `log_user` int(11) NOT NULL,
  `log_server` int(11) NOT NULL,
  `log_channel` int(11) NOT NULL,
  `log_data` int(11) NOT NULL,
  `log_result` int(11) NOT NULL,
  `log_time` datetime NOT NULL,
  `f1` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f2` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f3` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f4` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f5` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f6` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE `log_office` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `log_type` int(11) NOT NULL,
  `log_user` int(11) NOT NULL,
  `log_server` int(11) NOT NULL,
  `log_channel` int(11) NOT NULL,
  `log_data` int(11) NOT NULL,
  `log_result` int(11) NOT NULL,
  `log_time` datetime NOT NULL,
  `f1` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f2` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f3` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f4` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f5` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f6` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE `log_resource` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `log_type` int(11) NOT NULL,
  `log_user` int(11) NOT NULL,
  `log_server` int(11) NOT NULL,
  `log_channel` int(11) NOT NULL,
  `log_data` int(11) NOT NULL,
  `log_result` int(11) NOT NULL,
  `log_time` datetime NOT NULL,
  `f1` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f2` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f3` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f4` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f5` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f6` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE `log_local` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `log_type` int(11) NOT NULL,
  `log_user` int(11) NOT NULL,
  `log_server` int(11) NOT NULL,
  `log_channel` int(11) NOT NULL,
  `log_data` int(11) NOT NULL,
  `log_result` int(11) NOT NULL,
  `log_time` datetime NOT NULL,
  `f1` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f2` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f3` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f4` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f5` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f6` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE `log_food` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `log_type` int(11) NOT NULL,
  `log_user` int(11) NOT NULL,
  `log_server` int(11) NOT NULL,
  `log_channel` int(11) NOT NULL,
  `log_data` int(11) NOT NULL,
  `log_result` int(11) NOT NULL,
  `log_time` datetime NOT NULL,
  `f1` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f2` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f3` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f4` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f5` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f6` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE `log_arena` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `log_type` int(11) NOT NULL,
  `log_user` int(11) NOT NULL,
  `log_server` int(11) NOT NULL,
  `log_channel` int(11) NOT NULL,
  `log_data` int(11) NOT NULL,
  `log_result` int(11) NOT NULL,
  `log_time` datetime NOT NULL,
  `f1` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f2` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f3` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f4` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f5` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f6` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE `log_seige` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `log_type` int(11) NOT NULL,
  `log_user` int(11) NOT NULL,
  `log_server` int(11) NOT NULL,
  `log_channel` int(11) NOT NULL,
  `log_data` int(11) NOT NULL,
  `log_result` int(11) NOT NULL,
  `log_time` datetime NOT NULL,
  `f1` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f2` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f3` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f4` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f5` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f6` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE `log_king` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `log_type` int(11) NOT NULL,
  `log_user` int(11) NOT NULL,
  `log_server` int(11) NOT NULL,
  `log_channel` int(11) NOT NULL,
  `log_data` int(11) NOT NULL,
  `log_result` int(11) NOT NULL,
  `log_time` datetime NOT NULL,
  `f1` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f2` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f3` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f4` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f5` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f6` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE `log_upgrade` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `log_type` int(11) NOT NULL,
  `log_user` int(11) NOT NULL,
  `log_server` int(11) NOT NULL,
  `log_channel` int(11) NOT NULL,
  `log_data` int(11) NOT NULL,
  `log_result` int(11) NOT NULL,
  `log_time` datetime NOT NULL,
  `f1` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f2` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f3` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f4` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f5` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `f6` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;
