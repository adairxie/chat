create database if not exists chat;
use chat;

create table if not exists userinfo
(
    user_id int unsigned auto_increment primary key comment 'user id' ,
    user_name varchar(64) not null default '' comment 'user name' ,
    password  varchar(64) not null default '' comment 'password' ,
    status int not null default 0 comment 'user status' ,
		lasttime timestamp default current_timestamp on update current_timestamp
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='user table' ;

