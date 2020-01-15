#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <string>
#include <iostream>
#include <vector>
#include <hiredis/hiredis.h>
using namespace std;

class RedisContext
{
public:
    RedisContext();
    RedisContext(string IP, int Port);
    ~RedisContext();

    //操作函数

    bool IsKeyExist(string &key);
    int setString(string &key, string &value);
    string getString(string &key);
    int delString(string &key);

    int setList(string key, vector<int> value);
    vector<int> getList(string key);
    
    int scanKey(int cursor ,string pattern,int count);
    int sscanKey(string key,int cursor ,string pattern,int count);
    int hscanKey(string key,int cursor ,string pattern,int count);
    int zscanKey(string key,int cursor ,string pattern,int count);

private:
    //初始化
    bool init(string IP, int Port);
    //是否连接正常
    bool isError();
    //释放连接
    void freeReply();
    //断开连接
    bool disConnect();
    //连接Redis
    bool Connect();
    //重连Redis
    bool  ReConnect();
    //命令返回成功判断
    bool ReplyFlag(redisReply* rply);

    bool Reply();
    redisContext *m_redis;
    string m_IP;
    int m_Port;
};

RedisContext::RedisContext()
{
    m_redis = NULL;
    m_IP = "127.0.0.1";
    m_Port = 6379;

    bool bRet = init(m_IP, m_Port);

    if (false == bRet)
    {
        cout << "connect Redis failed,IP is:" << m_IP << ",Port is:" << m_Port << endl;
        m_redis = NULL;
    }

    cout << "connect Redis success,IP is:" << m_IP << ",Port is:" << m_Port << endl;
}

RedisContext::RedisContext(string IP, int Port)
{
    m_redis = NULL;
    m_IP = IP;
    m_Port = Port;

    bool bRet = init(m_IP, m_Port);

    if (false == bRet)
    {
        cout << "connect Redis failed,IP is:" << m_IP << ",Port is:" << m_Port << endl;
        m_redis = NULL;
    }

    cout << "connect Redis success,IP is:" << m_IP << ",Port is:" << m_Port << endl;
}

RedisContext::~RedisContext()
{
    if (m_redis != NULL)
    {
        redisFree(m_redis);//析构函数释放资源
        cout << "~RedisContext :: free redis connection " << endl;
    }
}

bool RedisContext::init(string IP, int Port)
{
    cout << "init : ip = " << IP << "port:" << Port << endl;
    m_redis = redisConnect(IP.c_str(), Port);
    if (m_redis == NULL || m_redis->err) {
        printf("RedisContext : Connection error: %s\n", m_redis->errstr);
    }
    else
    {
        cout << "init redis tool success " << endl;
    }
}

bool RedisContext::isError()
{
    if (NULL == m_redis)
    {
        return true;
    }
    return false;
}

bool RedisContext::ReConnect()
{
    if (NULL == m_redis)
    {
        disConnect();
        Connect();
        return true;
    }
    return false;
}

void RedisContext::freeReply()
{
    if (m_redis != NULL)
    {
        freeReplyObject(m_redis);
        m_redis = NULL;
    }
}

bool RedisContext::disConnect()
{
    if (m_redis != NULL)
    {
        freeReplyObject(m_redis);
        m_redis = NULL;
        return true;
    }

    return false;

}

bool RedisContext::Connect()
{
    bool bRet = init(m_IP, m_Port);

    if (false == bRet)
    {
        cout << "connect Redis failed,IP is:" << m_IP << ",Port is:" << m_Port << endl;
        m_redis = NULL;
    }

    cout << "connect Redis success,IP is:" << m_IP << ",Port is:" << m_Port << endl;
}

bool RedisContext::ReplyFlag(redisReply* rply)
{
    if (NULL == rply)
    {
        return false;
    }

    if (!(rply->type == REDIS_REPLY_STATUS && strcasecmp(rply->str, "OK") == 0))
    {
        return false;
    }

    return true;
}

int  RedisContext::setString(string &key, string &value)
{
    if (m_redis == NULL || m_redis->err)//int err; /* Error flags, 错误标识，0表示无错误 */
    {
        cout << "Redis init Error !!!" << endl;
        ReConnect();
        return -1;
    }
    redisReply *reply;
    reply = (redisReply *)redisCommand(m_redis, "SET %s %s", key.c_str(), value.c_str());//执行写入命令
    cout << "set string type = " << reply->type << endl;//获取响应的枚举类型
    int result = 0;
    if (reply == NULL)
    {
        redisFree(m_redis);
        m_redis = NULL;
        result = -1;
        cout << "set string fail : reply->str = NULL " << endl;
        //pthread_spin_unlock(&m_redis_flock);
        return -1;
    }
    else if (strcmp(reply->str, "OK") == 0)//根据不同的响应类型进行判断获取成功与否
    {
        result = 1;
    }
    else
    {
        result = -1;
        cout << "set string fail :" << reply->str << endl;
    }
    freeReplyObject(reply);//释放响应信息

    return result;
}

//向数据库写入vector（list）类型数据
int RedisContext::setList(string key, vector<int> value)
{
    if (m_redis == NULL || m_redis->err)
    {
        cout << "Redis init Error !!!" << endl;
        ReConnect();
        return -1;
    }

    redisReply *reply;

    int valueSize = value.size();
    int result = 0;

    for (int i = 0; i < valueSize; i++)
    {
        reply = (redisReply*)redisCommand(m_redis, "RPUSH %s %d", key.c_str(), value.at(i));
        cout << "set list type = " << reply->type << " ";
        int old = reply->integer;
        if (reply == NULL)
        {
            redisFree(m_redis);
            m_redis = NULL;
            result = -1;
            cout << "set list fail : reply->str = NULL " << endl;
            //pthread_spin_unlock(&m_redis_flock);
            return -1;
        }
        else if (reply->integer == old++)
        {
            result = 1;
            cout << "rpush list ok" << endl;
            continue;
        }
        else
        {
            result = -1;
            cout << "set list fail ,reply->integer = " << reply->integer << endl;
            return -1;
        }

    }

    freeReplyObject(reply);
    cout << "set List  success" << endl;
    return result;

}

//从数据库读出vector（list）类型数据
vector<int> RedisContext::getList(string key)
{
    vector<int>v_out;
    v_out.clear();
    if (m_redis == NULL || m_redis->err)
    {
        cout << "Redis init Error !!!" << endl;
        ReConnect();
        return v_out;//返回空的向量
    }

    redisReply *reply;
    reply = (redisReply*)redisCommand(m_redis, "LLEN %s", key.c_str());
    int valueSize = reply->integer;
    cout << "List size is :" << reply->integer << endl;

    reply = (redisReply*)redisCommand(m_redis, "LRANGE %s %d %d", key.c_str(), 0, valueSize - 1);
    cout << "get list type = " << reply->type << endl;
    cout << "get list size = " << reply->elements << endl;//对于数组类型可以用elements元素获取数组长度

    redisReply** replyVector = reply->element;//获取数组指针
    vector<int> result;
    for (int i = 0; i < valueSize; i++)
    {
        string temp = (*replyVector)->str;//遍历redisReply*数组,存入vector向量
        int a = atoi(temp.c_str());
        result.push_back(a);
        replyVector++;
    }

    cout << "result size:" << result.size() << endl;
    
    freeReplyObject(r);
    return result;

}

int RedisContext::scanKey(int cursor ,string pattern,int count)
{
    string command="scan "+to_string(cursor)+" match "+pattern+" count "+to_string(count);
    r = (redisReply*)redisCommand(c, command);
    printf("command execute command[%s].\n", command);
    if(NULL==r)
    {
        printf("command reply is NULL\n");
        freeReplyObject(r);
        redisFree(c);
        return ;
    }
    
    int index = atoi(r->element[0]->str);
    printf("index:%d\n",index);
    if(1 == r->elements)
    {
        printf("no data");
        return;
    }
    if (r->element[1]->type != REDIS_REPLY_ARRAY)
    {
        printf("redis scan keys reply not array");
        freeReplyObject(r);
        return;
    }
    
    printf("r->element[1]->elements is %d\n",r->element[1]->elements);
    for (int i = 0; i < r->element[1]->elements; i++) 
    {
        if(r->element[1]->element[i]==NULL || r->element[1]->element[i]->str==NULL)
        {
            printf("err\n");
            return ;
        }
        printf("i:%d,key:%s\n",i,r->element[1]->element[i]->str);
    }

    freeReplyObject(r);
}

bool RedisContext::IsKeyExist(string &key)
{
    if (m_redis == NULL || m_redis->err)//int err; /* Error flags, 错误标识，0表示无错误 */
    {
        cout << "Redis init Error !!!" << endl;
        ReConnect();
        return false;
    }
    redisReply *reply;
    reply = (redisReply *)redisCommand(m_redis, "EXISTS %s", key.c_str());//执行写入命令
    if (reply == NULL)
    {
        redisFree(m_redis);
        m_redis = NULL;
        cout << "IsKeyExist fail : reply->str = NULL " << endl;
        //pthread_spin_unlock(&m_redis_flock);
        return false;
    }
    else if (strcmp(reply->str, "1") == 0)//根据不同的响应类型进行判断获取成功与否
    {
        cout << "key Exist :" << reply->str << endl;
        freeReplyObject(reply);//释放响应信息
        return true;
    }
    else
    {
        cout << "key No Exist :" << reply->str << endl;
        freeReplyObject(reply);//释放响应信息
        return false;
    }
    freeReplyObject(reply);//释放响应信息

    return true;
}
void doTest()
{
    //该对象将用于其后所有与Redis操作的函数。
    redisContext* c = redisConnect("127.0.0.1", 6379);
    if (c->err) {
        printf("error to execute command[%s].\n", c->errstr);
        redisFree(c);
        return;
    }
    const char* command1 = "set stest1 value1";
    redisReply* r = (redisReply*)redisCommand(c, command1);
    //需要注意的是，如果返回的对象是NULL，则表示客户端和服务器之间出现严重错误，必须重新链接。
    //这里只是举例说明，简便起见，后面的命令就不再做这样的判断了。
    if (NULL == r) {
        redisFree(c);
        return;
    }
    //不同的Redis命令返回的数据类型不同，在获取之前需要先判断它的实际类型。
    //至于各种命令的返回值信息，可以参考Redis的官方文档，或者查看该系列博客的前几篇
    //有关Redis各种数据类型的博客。:)
    //字符串类型的set命令的返回值的类型是REDIS_REPLY_STATUS，然后只有当返回信息是"OK"
    //时，才表示该命令执行成功。后面的例子以此类推，就不再过多赘述了。
    if (!(r->type == REDIS_REPLY_STATUS && strcasecmp(r->str, "OK") == 0)) {
        printf("Failed to execute command[%s].\n", command1);
        freeReplyObject(r);
        redisFree(c);
        return;
    }
    //由于后面重复使用该变量，所以需要提前释放，否则内存泄漏。
    freeReplyObject(r);
    printf("Succeed to execute command[%s].\n", command1);

    const char* command2 = "strlen stest1";
    r = (redisReply*)redisCommand(c, command2);
    if (r->type != REDIS_REPLY_INTEGER) {
        printf("Failed to execute command[%s].\n", command2);
        freeReplyObject(r);
        redisFree(c);
        return;
    }
    int length = r->integer;
    freeReplyObject(r);
    printf("The length of 'stest1' is %d.\n", length);
    printf("Succeed to execute command[%s].\n", command2);

    const char* command3 = "get stest1";
    r = (redisReply*)redisCommand(c, command3);
    if (r->type != REDIS_REPLY_STRING) {
        printf("Failed to execute command[%s].\n", command3);
        freeReplyObject(r);
        redisFree(c);
        return;
    }
    printf("The value of 'stest1' is %s.\n", r->str);
    freeReplyObject(r);
    printf("Succeed to execute command[%s].\n", command3);

    const char* command4 = "get stest2";
    r = (redisReply*)redisCommand(c, command4);
    //这里需要先说明一下，由于stest2键并不存在，因此Redis会返回空结果，这里只是为了演示。
    if (r->type != REDIS_REPLY_NIL) {
        printf("Failed to execute command[%s].\n", command4);
        freeReplyObject(r);
        redisFree(c);
        return;
    }
    freeReplyObject(r);
    printf("Succeed to execute command[%s].\n", command4);
    
    const char* command6 = "scan 320 match * COUNT 10";
    r = (redisReply*)redisCommand(c, command6);
    //这里需要先说明一下，由于stest2键并不存在，因此Redis会返回空结果，这里只是为了演示。
    printf("command6 execute command[%s].\n", command6);
    if(NULL==r)
    {
        printf("command6 reply is NULL\n");
        freeReplyObject(r);
        redisFree(c);
        return ;
    }
    
    int index = atoi(r->element[0]->str);
    printf("index:%d\n",index);
    if(1 == r->elements)
    {
        printf("no data");
        return;
    }
    if (r->element[1]->type != REDIS_REPLY_ARRAY)
    {
        printf("redis scan keys reply not array");
        freeReplyObject(r);
        return;
    }
    
    printf("r->element[1]->elements is %d\n",r->element[1]->elements);
    for (int i = 0; i < r->element[1]->elements; i++) 
    {
        if(r->element[1]->element[i]==NULL || r->element[1]->element[i]->str==NULL)
        {
            printf("err\n");
            return ;
        }
        printf("i:%d,key:%s\n",i,r->element[1]->element[i]->str);
    }

    freeReplyObject(r);

    const char* command5 = "mget stest1 stest2";
    r = (redisReply*)redisCommand(c, command5);
    //不论stest2存在与否，Redis都会给出结果，只是第二个值为nil。
    //由于有多个值返回，因为返回应答的类型是数组类型。
    if (r->type != REDIS_REPLY_ARRAY) {
        printf("Failed to execute command[%s].\n", command5);
        freeReplyObject(r);
        redisFree(c);
        //r->elements表示子元素的数量，不管请求的key是否存在，该值都等于请求是键的数量。
        assert(2 == r->elements);
        return;
    }
    for (int i = 0; i < r->elements; ++i) {
        redisReply* childReply = r->element[i];
        //之前已经介绍过，get命令返回的数据类型是string。
        //对于不存在key的返回值，其类型为REDIS_REPLY_NIL。
        if (childReply->type == REDIS_REPLY_STRING)
            printf("The value is %s.\n", childReply->str);
    }
    //对于每一个子应答，无需使用者单独释放，只需释放最外部的redisReply即可。
    freeReplyObject(r);
    printf("Succeed to execute command[%s].\n", command5);

    printf("Begin to test pipeline.\n");
    //该命令只是将待发送的命令写入到上下文对象的输出缓冲区中，直到调用后面的
    //redisGetReply命令才会批量将缓冲区中的命令写出到Redis服务器。这样可以
    //有效的减少客户端与服务器之间的同步等候时间，以及网络IO引起的延迟。
    //至于管线的具体性能优势，可以考虑该系列博客中的管线主题。
    if (REDIS_OK != redisAppendCommand(c, command1)
        || REDIS_OK != redisAppendCommand(c, command2)
        || REDIS_OK != redisAppendCommand(c, command3)
        || REDIS_OK != redisAppendCommand(c, command4)
        || REDIS_OK != redisAppendCommand(c, command5)) {
        redisFree(c);
        return;
    }

    redisReply* reply = NULL;
    //对pipeline返回结果的处理方式，和前面代码的处理方式完全一直，这里就不再重复给出了。
    if (REDIS_OK != redisGetReply(c, (void**)&reply)) {
        printf("Failed to execute command[%s] with Pipeline.\n", command1);
        freeReplyObject(reply);
        redisFree(c);
    }
    freeReplyObject(reply);
    printf("Succeed to execute command[%s] with Pipeline.\n", command1);

    if (REDIS_OK != redisGetReply(c, (void**)&reply)) {
        printf("Failed to execute command[%s] with Pipeline.\n", command2);
        freeReplyObject(reply);
        redisFree(c);
    }
    freeReplyObject(reply);
    printf("Succeed to execute command[%s] with Pipeline.\n", command2);

    if (REDIS_OK != redisGetReply(c, (void**)&reply)) {
        printf("Failed to execute command[%s] with Pipeline.\n", command3);
        freeReplyObject(reply);
        redisFree(c);
    }
    freeReplyObject(reply);
    printf("Succeed to execute command[%s] with Pipeline.\n", command3);

    if (REDIS_OK != redisGetReply(c, (void**)&reply)) {
        printf("Failed to execute command[%s] with Pipeline.\n", command4);
        freeReplyObject(reply);
        redisFree(c);
    }
    freeReplyObject(reply);
    printf("Succeed to execute command[%s] with Pipeline.\n", command4);

    if (REDIS_OK != redisGetReply(c, (void**)&reply)) {
        printf("Failed to execute command[%s] with Pipeline.\n", command5);
        freeReplyObject(reply);
        redisFree(c);
    }
    freeReplyObject(reply);
    printf("Succeed to execute command[%s] with Pipeline.\n", command5);
    //由于所有通过pipeline提交的命令结果均已为返回，如果此时继续调用redisGetReply，
    //将会导致该函数阻塞并挂起当前线程，直到有新的通过管线提交的命令结果返回。
    //最后不要忘记在退出前释放当前连接的上下文对象。
    redisFree(c);
    return;
}

int main()
{
    doTest();
    return 0;
}
