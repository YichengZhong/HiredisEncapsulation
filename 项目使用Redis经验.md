## 获取redis的rdb目录 ##

redis-cli
 
127.0.0.1:6379>  CONFIG GET dir
1) "dir"
2) "/var/lib/redis"

redis面试

Redis支持哪几种数据类型？
支持多种类型的数据结构
1.string：最基本的数据类型，二进制安全的字符串，最大512M。
2.list：按照添加顺序保持顺序的字符串列表。
3.set：无序的字符串集合，不存在重复的元素。
4.sorted set：已排序的字符串集合。
5.hash：key-value对的一种集合。

Redis有哪几种数据淘汰策略？
在Redis中，允许用户设置最大使用内存大小server.maxmemory，当Redis 内存数据集大小上升到一定大小的时候，就会施行数据淘汰策略。
1.volatile-lru:从已设置过期的数据集中挑选最近最少使用的淘汰
2.volatile-ttr:从已设置过期的数据集中挑选将要过期的数据淘汰
3.volatile-random:从已设置过期的数据集中任意挑选数据淘汰
4.allkeys-lru:从数据集中挑选最近最少使用的数据淘汰
5.allkeys-random:从数据集中任意挑选数据淘汰
6.noenviction:禁止淘汰数据
redis淘汰数据时还会同步到aof