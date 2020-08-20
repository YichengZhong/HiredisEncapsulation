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