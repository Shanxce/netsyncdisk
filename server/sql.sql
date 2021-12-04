-- --------------------------------------------------------
-- 主机:                           1.15.189.31
-- 服务器版本:                        10.3.28-MariaDB - MariaDB Server
-- 服务器操作系统:                      Linux
-- HeidiSQL 版本:                  11.0.0.5919
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;


-- 导出 db1851506 的数据库结构
CREATE DATABASE IF NOT EXISTS `db1851506` /*!40100 DEFAULT CHARACTER SET latin1 */;
USE `db1851506`;

-- 导出  表 db1851506.file_md5_info 结构
CREATE TABLE IF NOT EXISTS `file_md5_info` (
  `file_id` int(11) NOT NULL AUTO_INCREMENT,
  `file_md5` char(40) DEFAULT NULL,
  `file_name` text NOT NULL,
  `file_parent` int(11) NOT NULL,
  `file_modifytime` int(11) NOT NULL,
  PRIMARY KEY (`file_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1049 DEFAULT CHARSET=gbk;

-- 数据导出被取消选择。

-- 导出  表 db1851506.file_num 结构
CREATE TABLE IF NOT EXISTS `file_num` (
  `file_md5` char(40) NOT NULL,
  `file_num` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=gbk;

-- 数据导出被取消选择。

-- 导出  表 db1851506.folder_info 结构
CREATE TABLE IF NOT EXISTS `folder_info` (
  `folder_info_id` int(11) NOT NULL AUTO_INCREMENT,
  `user_no` char(25) NOT NULL,
  `addr_server` char(255) NOT NULL DEFAULT '0',
  `addr_local` char(255) DEFAULT NULL,
  `isbind` bit(1) NOT NULL DEFAULT b'0',
  `cp_ip` char(40) DEFAULT NULL,
  PRIMARY KEY (`folder_info_id`)
) ENGINE=InnoDB AUTO_INCREMENT=326 DEFAULT CHARSET=gbk;

-- 数据导出被取消选择。

-- 导出  表 db1851506.user_info 结构
CREATE TABLE IF NOT EXISTS `user_info` (
  `user_id` int(11) NOT NULL AUTO_INCREMENT,
  `user_no` char(25) NOT NULL,
  `user_passwd` char(40) NOT NULL DEFAULT '',
  PRIMARY KEY (`user_id`)
) ENGINE=InnoDB AUTO_INCREMENT=25 DEFAULT CHARSET=gbk;

-- 数据导出被取消选择。

/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
