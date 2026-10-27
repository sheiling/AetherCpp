#ifndef COMMON_CORE_DEVICE_MOTION_CONTROL_MOTION_TYPES_H
#define COMMON_CORE_DEVICE_MOTION_CONTROL_MOTION_TYPES_H

#include <map>
#include <string>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <Logger.h>
#include <common/utility/json_tool/reflector.h>
namespace common
{
namespace device
{

namespace axis
{

enum class HomeModeType {
    Origin_FindHome,			// 0:原点回零
    Z_FindHome,					// 1:Z相回零
    Limit_FindHome,				// 2:限位回零
    Origin_Add_Z_FindHome,		// 3:原点+Z相回零
    Limit_Add_Z_FindHome,		// 4:限位+Z相回零
    Current_Position,			// 5:以当前位置为零位
    Enable						// 6:使能后去到零位
};

NLOHMANN_JSON_SERIALIZE_ENUM(HomeModeType, {
{ HomeModeType::Origin_FindHome, "Origin_FindHome" },
{ HomeModeType::Z_FindHome, "Z_FindHome" },
{ HomeModeType::Limit_FindHome, "Limit_FindHome" },
{ HomeModeType::Origin_Add_Z_FindHome, "Origin_Add_Z_FindHome" },
{ HomeModeType::Limit_Add_Z_FindHome, "Limit_Add_Z_FindHome" },
{ HomeModeType::Current_Position, "Current_Position" },
{ HomeModeType::Enable, "Enable" }
    });

class Axis final
{
public:
    Axis() {}
    Axis& operator=(const Axis& other) { return *this; };
    Axis(const Axis& other) {}
    Axis(const Axis&& other) noexcept {}

    //位置比较输出操作类型
    enum class AxisPsoAction
    {
        WriteIO_High,		//IO 置为高电平
        WriteIO_Low,		//IO 置为低电平
        WriteIO_Reverse,	//IO 电平翻转
        WriteIO_500us,		//IO 输出 500us 脉冲
        WriteIO_1ms,		//IO 输出 1ms 脉冲
        WriteIO_10ms,		//IO 输出 10ms 脉冲
        WriteIO_100ms,		//IO 输出 100ms 脉冲
        StopAxis,			//停止指定轴
        ChangeSpeed,		//在线变速
    };

    //位置锁存参数
    struct AxisLtc
    {
        AxisLtc() = default;
        AxisLtc(bool enable) : m_enable(enable) {}
        AxisLtc(bool enable, int softLtc) : m_enable(enable), m_softLtc(softLtc) {}
        AxisLtc(bool enable, int softLtc, int ltcNo) : m_enable(enable), m_softLtc(softLtc), m_ltcNo(ltcNo) {}
        AxisLtc(bool enable, int softLtc, int ltcNo, int mode) : m_enable(enable), m_softLtc(softLtc), m_ltcNo(ltcNo), m_mode(mode) {}
        AxisLtc(bool enable, int softLtc, int ltcNo, int mode, int ltcSrc) : m_enable(enable), m_softLtc(softLtc), m_ltcNo(ltcNo), m_mode(mode), m_ltcSrc(ltcSrc) {}
        AxisLtc(bool enable, int softLtc, int ltcNo, int mode, int ltcSrc, int inBit, int logic, int filter) :
            m_enable(enable), m_softLtc(softLtc), m_ltcNo(ltcNo), m_mode(mode), m_ltcSrc(ltcSrc), m_inBit(inBit), m_logic(logic), m_filter(filter) {}

        bool m_enable = false;			//使能状态，false：关闭，true：开启
        int m_softLtc = 0;				//锁存器选择，0：软件锁存，1：高速锁存
        int m_ltcNo = 0;				//锁存器编号，0-3
        int m_mode = 0;					//锁存模式，0：单次锁存，1：连续锁存
        int m_ltcSrc = 0;				//锁存源，0-指令位置，1-编码器反馈位置
        int m_inBit = 0;				//锁存触发信号
        int m_logic = 0;				//触发边沿逻辑，0-下降沿，1-上升沿，2-双边沿
        int m_filter = 0;				//滤波时间，单位：ms
    };
    //软限位设置参数
    struct AxisSoftlimit
    {
        AxisSoftlimit() = default;
        AxisSoftlimit(bool enable) : m_enable(enable) {}
        AxisSoftlimit(bool enable, int sourceSel, int slAction, double limitN, double limitP) :
            m_enable(enable), m_sourceSel(sourceSel), m_slAction(slAction), m_limitN(limitN), m_limitP(limitP) {}

        bool m_enable = false;		//使能状态，false：关闭，true：开启
        int m_sourceSel = 0;		//计数器选择，0：指令位置计数器，1：编码器计数器
        int m_slAction = 0;			//限位停止方式，0：立即停止，1： 减速停止
        double m_limitN = 0.0;		//负限位位置，单位：unit
        double m_limitP = 0.0;		//正限位位置，单位：unit
    };
    //回零参数设置
    struct AxisHome
    {
        AxisHome() = default;

        uint32_t m_homeDir = 1;										// 回原点方向(0-负向, 1-正向)
        uint32_t m_homeEdgeType = 3;								// 回原点边缘类型 详细定义见getHomeMode
        uint32_t m_homeVelMode = 1;									// 回原速度模式(0：低速回原, 1：高速回原)
        int m_homePinLogic = -1;									// 原点有效电平(-1:不设置, 0：低有效, 1：高有效)
        int m_homeSourceSel = 1;									// 回零计数源，0：指令位置计数器，1：编码器计数器
        HomeModeType m_homeMode = HomeModeType::Origin_FindHome;	// 回原点模式(0:原点回零, 1:Z相回零, 2:限位回零, 3:原点+Z相回零, 4:限位+Z相回零, 5:以当前位置为零位, 6:使能后去到零位)
        double m_homeOffset = 0;									// 零点偏移量（正值正向偏移；负值负向偏移；0 无偏移）。正限位回零只 能设置负向偏移，负限位回零只能设置正向偏移, 单位脉冲unit
        double m_homeHighVel = 50;									// 回零高速
        double m_homeLowVel = 5;									// 回零低速
        double m_homeTacc = 0.5;									// 回零加速度
    };
    //PDO读写参数
    struct AxisPDO
    {
        AxisPDO() = default;
        AxisPDO(bool isUnsigned, bool isFloat, int index, int subIndex) :
            m_isUnsigned(isUnsigned), m_isFloat(isFloat), m_index(index), m_subIndex(subIndex) {
        }

        bool m_isUnsigned = true;	//是否有符号，false：有符号，true：无符号
        bool m_isFloat = false;		//是否为浮点数，false：不是，true：是浮点数
        int m_port = 2;				//总线端口号
        int m_index = 0;			//寄存器地址
        int m_subIndex = 0;			//子索引
        int m_length = 32;			//位数
    };
    //轴状态
    struct AxisSta
    {
        bool ALM = false;			//伺服报警
        bool ELP = false;			//正硬限位
        bool ELN = false;			//负硬限位
        bool EMG = false;			//急停
        bool ORG = false;			//原点
        bool SLP = false;			//正软限位
        bool SLN = false;			//负软限位
        bool INP = false;			//伺服到位
        bool EZ = false;			//EZ 信号
        bool RDY = false;			//就绪
        bool DSTP = false;			//减速停止
        bool MOVE_DIR = false;		//轴运动方向
        unsigned short  RUN_MODE = 0;//轴运动模式0：空闲1：Pmove 2：Vmove 3：Hmove 4：Handwheel 5：Ptt / Pts 6：Pvt / Pvts 7：Gear 8：Cam 9：Line 10：Continue
    };
    //轴状态有效电平
    struct AxisStaLogic
    {
        int m_orgLogic;				//0:低有效, 1:高有效
    };


    //运动参数
    struct MoveParameters
    {
        double m_startVel = 2;					// 起始速度
        double m_stopVel = 2;					// 停止速度
        double m_maxVel = 50;					// 最大速度
        double m_tacc = 0.5;                    // 加速度
        double m_tdec = 0.5;                    // 减速度
        double m_sPara = 0.2;                   // S形平滑系数

        void reflectFromJson(const json& j);
        void reflectToJson(json& j)const;
    };
    /** 轴运行时过程数据（不持久化，经 AxisSta 对外暴露） */
    struct AxisProcessData
    {
        bool           m_moveDir = false;  // 轴运动方向
        unsigned short m_runMode = 0;      // 0:空闲 1:Pmove 2:Vmove 3:Hmove …
    };
    //轴参数
    struct AxisData
    {
        bool m_enable = true;										// 开启轴(false：关闭轴，true：开启轴)
        int  m_axisNO = 0;											// 轴编号
        int  m_slaveID = 0;											// 从站 ID（总线轴 PDO 访问用；0 表示驱动内按 1000+axisNO 回退）
        int  m_axisType = 1;										// 轴类型(0:直线电机, 1:伺服电机, 2:步进电机)
        int  m_unitType = 0;										// 轴位移单位类型(0:脉冲, 1:毫米, 2:微米, 3:角度)
        int  m_encoderType = 0;										// 编码器类型(0:普通增量式编码器, 1:多圈绝对式编码器)
        int  m_driverType = 0;										// 驱动器类型(0:默认,1:松下)
        std::string m_cardName = "CardOne";							// 所属控制卡的名称
        std::string m_moduleName = "";								// 所属模块名称
        uint32_t m_homeDir = 1;										// 回原点方向(0-负向, 1-正向)
        uint32_t m_homeEdgeType = 3;								// 回原点边缘类型 详细定义见getHomeMode
        uint32_t m_homeVelMode = 1;									// 回原速度模式(0：低速回原, 1：高速回原)
        int m_homePinLogic = -1;									// 原点有效电平(-1:不设置, 0：低有效, 1：高有效)
        int m_homeSourceSel = 1;									// 回零计数源，0：指令位置计数器，1：编码器计数器
        HomeModeType m_homeMode = HomeModeType::Origin_FindHome;	// 回原点模式(0:原点回零, 1:Z相回零, 2:限位回零, 3:原点+Z相回零, 4:限位+Z相回零, 5:以当前位置为零位, 6:使能后去到零位)
        double m_homeOffset = 0;									// 零点偏移量（正值正向偏移；负值负向偏移；0 无偏移）。正限位回零只 能设置负向偏移，负限位回零只能设置正向偏移, 单位脉冲unit
        double m_homeHighVel = 50;									// 回零高速
        double m_homeLowVel = 5;									// 回零低速
        double m_homeTacc = 0.5;									// 回零加速度
        double m_equiv = -1;										// 脉冲当量 (-1:不设置,使用默认值)
        double m_pulseUnit = 1;										// 位移距离转换为脉冲数量的精度转换单位
        double m_negativeLimit = 0.0;								// 负限位位置，单位：unit
        double m_positiveLimit = 0.0;								// 正限位位置，单位：unit
        double m_errorBandPluse = 0.0;								// 设置轴运动到位的偏差范围，单位：unit
        int m_sourceSelLimit = 1;									// 软限位计数器选择(0:指令位置计数器, 1:编码器位置计数器)
        int m_softwareLimitAction = 0;								// 软限位停止方式(0:立即停止, 1:减速停止)

        void reflectFromJson(const json& j);
        void reflectToJson(json& j)const;
    };

    void reflectFromJson(const json& j);
    void reflectToJson(json& j)const;

    void setAxisParams(const AxisData& axisData)
    {
        std::lock_guard<std::mutex> lock(m_mutexAxis);
        axisDafaultParams = axisData;
    }
    AxisData getAxisParams()
    {
        std::lock_guard<std::mutex> lock(m_mutexAxis);
        return axisDafaultParams;
    }

    void setMoveParameters(const MoveParameters& axisData)
    {
        std::lock_guard<std::mutex> lock(m_mutexAxis);
        axisMoveParams = axisData;
    }
    MoveParameters getMoveParameters()
    {
        std::lock_guard<std::mutex> lock(m_mutexAxis);
        return axisMoveParams;
    }

    void setAxisProcessData(const AxisProcessData& data)
    {
        std::lock_guard<std::mutex> lock(m_mutexAxis);
        axisProcessData = data;
    }
    AxisProcessData getAxisProcessData()
    {
        std::lock_guard<std::mutex> lock(m_mutexAxis);
        return axisProcessData;
    }

private:
    AxisData axisDafaultParams;
    MoveParameters axisMoveParams;
    AxisProcessData axisProcessData;
    std::mutex m_mutexAxis;
};
}
namespace deviceIO
{
class DeviceIO
{

public:
    DeviceIO() {}
    DeviceIO(const DeviceIO& other) {}
    DeviceIO& operator=(const DeviceIO& other) { return *this; };
    DeviceIO(const DeviceIO&& other) {}

    struct DeviceIoData
    {
        std::string m_cardName = "CardOne";
        std::string m_moduleName = "";		//模块名称 业务层用来分组展示
        uint16_t m_node = 0;				//节点号  扩展模块才有节点号，本地IO可忽略
        uint16_t m_port = 1;				//端口号
        uint16_t m_logic = 0;               //0表示低电平有效，1表示高电平有效
        uint16_t m_type = 0;                //0表示输入IO，1表示输出IO
        uint16_t m_extend = 0;				//0表示控制卡，1表示扩展卡

        void reflectFromJson(const json& j) {
            getFromJson(j, "cardName", m_cardName);
            getFromJson(j, "moduleName", m_moduleName);
            getFromJson(j, "node", m_node);
            getFromJson(j, "port", m_port);
            getFromJson(j, "logic", m_logic);
            getFromJson(j, "type", m_type);
            getFromJson(j, "extend", m_extend);
        }
        void reflectToJson(json& j) const {
            j["cardName"] = m_cardName;
            j["moduleName"] = m_moduleName;
            j["node"] = m_node;
            j["port"] = m_port;
            j["logic"] = m_logic;
            j["type"] = m_type;
            j["extend"] = m_extend;
        }
    };

    void reflectFromJson(const json& j) {
        m_ioData.reflectFromJson(j);
    }
    void reflectToJson(json& j)const {
        m_ioData.reflectToJson(j);
    }

    void setDeviceIoData(const DeviceIoData& ioData)
    {
        std::lock_guard<std::mutex> lock(m_mutexData);
        m_ioData = ioData;
    }
    DeviceIoData getDeviceIoData()
    {
        std::lock_guard<std::mutex> lock(m_mutexData);
        return m_ioData;
    }

private:
    DeviceIoData m_ioData;
    std::mutex m_mutexData;
};
}
namespace deviceAD
{
class DeviceAD
{

public:
    DeviceAD() {}
    DeviceAD(const DeviceAD& other) {}
    DeviceAD& operator=(const DeviceAD& other) { return *this; };

    struct DeviceAdData
    {
        std::string m_cardName = "CardOne";
        uint16_t m_node = 0;				//节点号
        uint16_t m_channel = 0;				//通道号
        uint16_t m_type = 0;                //0表示输入(AD转换)， 1表示输出(DA转换)

        void reflectFromJson(const json& j) {
            getFromJson(j, "cardName", m_cardName);
            getFromJson(j, "node", m_node);
            getFromJson(j, "channel", m_channel);
            getFromJson(j, "type", m_type);
        }
        void reflectToJson(json& j)const {
            j["cardName"] = m_cardName;
            j["node"] = m_node;
            j["channel"] = m_channel;
            j["type"] = m_type;
        }
    };

    void reflectFromJson(const json& j) {
        m_adData.reflectFromJson(j);
    }
    void reflectToJson(json& j)const {
        m_adData.reflectToJson(j);
    }

    void setDeviceAdData(const DeviceAdData& adData)
    {
        std::lock_guard<std::mutex> lock(m_mutexData);
        m_adData = adData;
    }
    DeviceAdData getDeviceAdData()
    {
        std::lock_guard<std::mutex> lock(m_mutexData);
        return m_adData;
    }

private:
    DeviceAdData m_adData;
    std::mutex m_mutexData;
};
}
namespace motion
{
enum MotionCardDevType {
    LeiShineNmc = 0,                // 雷赛总线调用
    LeiShineDmc,					// 雷赛脉冲调用
    Solid,							// 升立德
    GaoChuanIO,						// 高川 IO 模块
    VMC,							// 虚拟运控
    MC_UNKNOW
};
NLOHMANN_JSON_SERIALIZE_ENUM(MotionCardDevType, {
{ MotionCardDevType::LeiShineNmc,         "LeiShineNmc" },
{ MotionCardDevType::LeiShineDmc,         "LeiShineDmc" },
{ MotionCardDevType::Solid,				  "Solid"		},
{ MotionCardDevType::GaoChuanIO,		  "GaoChuanIO"	},
{ MotionCardDevType::VMC,				  "VMC"			},
{ MotionCardDevType::MC_UNKNOW,			  "MC_UNKNOW"	}
    });

/** 轴/坐标系速度曲线（驱动层与门面层共用） */
struct AxisVelocityProfile {
    double startVel = 0.0;	// 起始速度，单位：unit/s
    double maxVel = 0.0;		// 最大速度，单位：unit/s
    double tacc = 0.5;		// 加速时间，单位：s
    double tdec = 0.5;		// 减速时间，单位：s
    double endVel = 0.0;		// 停止速度，单位：unit/s
};

/** 单轴定长运动参数（轴号由调用方单独传入） */
struct AxisSingleMoveParams {
    double dist = 0.0;			// 目标位移，单位：脉冲 unit（门面层已按 pulseUnit 换算）
    int posiMode = 0;			// 定位模式：0 相对运动，1 绝对运动
    AxisVelocityProfile profile;	// 本次运动速度曲线
    double smth = 0.0;			// S 形平滑系数（部分驱动使用，0 表示不启用）
};

/** 控制器 / 门面层单轴运动参数（轴名由调用方单独传入） */
struct AxisMoveParams {
    double dist = 0.0;			// 目标位移，单位：工程单位（由轴 pulseUnit 换算为脉冲）
    uint32_t posiMode = 0;		// 定位模式：0 相对运动，1 绝对运动
    double speed = 0.0;			// 本次运动最大速度，单位：unit/s；≤0 时使用轴默认 maxVel
    bool wait = true;			// 是否阻塞等待运动完成
    int timeout = 30000;		// wait 为 true 时的等待超时，单位：ms
};

/** 多轴直线插补运动参数（轴名列表由调用方单独传入） */
struct AxisLinearParams {
    std::vector<double> dists;	// 各轴目标位移，与 axisNames 一一对应，单位：工程单位
    uint32_t posiMode = 0;		// 定位模式：0 相对运动，1 绝对运动
    int crdNo = 0;				// 坐标系编号（插补坐标系号）
    int timeout = 30000;		// 等待插补完成的超时，单位：ms
};

/** 位置比较通道：低速比较可绑定任意 DO；高速比较绑定硬件 CMP 端口 */
enum class PositionCompareChannel {
    LowSpeed = 0,	// 低速一维位置比较（smc_compare_*）
    HighSpeed0 = 1,	// 高速比较器 0（硬件 OUT16 / CMP0）
    HighSpeed1 = 2,	// 高速比较器 1（硬件 OUT17 / CMP1）
};

/** 位置比较参考轴（直线插补 X-Y 平面内选择沿哪根轴判定位移） */
enum class PositionCompareRefAxis {
    X = 0,
    Y = 1,
};

/** 位置比较位置源 */
enum class PositionCompareSource {
    CommandPosition = 0,	// 指令位置计数器
    EncoderPosition = 1,	// 编码器反馈位置
};

/** 位置比较输出配置（低速比较用 ioName；高速比较用 cmpLogic / pulseWidthMs） */
struct PositionCompareOutputConfig {
    std::string ioName;		// 低速比较：DeviceIO 配置中的输出 IO 名称
    axis::Axis::AxisPsoAction action = axis::Axis::AxisPsoAction::WriteIO_Low;	// 低速比较触发动作
    uint16_t cmpLogic = 0;		// 高速比较有效电平：0 低电平，1 高电平
    uint16_t pulseWidthMs = 1;	// 高速比较队列模式输出脉冲宽度，单位：ms
};

/** 驱动层位置比较配置（轴号由调用方单独传入） */
struct PositionCompareDriverSetup {
    bool highSpeed = true;			// true：高速比较器；false：低速比较
    int channelNo = 0;				// 高速比较器编号 0-1；低速比较忽略
    int mode = 4;					// 比较模式：4 队列，0 禁止
    int compareSource = 1;			// 0 指令位置，1 编码器位置
    int cmpLogic = 0;				// 高速比较有效电平
    long pulseWidthUs = 1000;		// 高速比较脉冲宽度，单位：us
    int lowSpeedDir = 1;			// 低速比较方向：0 小于等于，1 大于等于
    axis::Axis::AxisPsoAction lowSpeedAction = axis::Axis::AxisPsoAction::WriteIO_Low;
    int lowSpeedIoBit = 0;			// 低速比较输出 IO 端口号
    std::vector<double> positions;	// 比较点绝对位置，单位：脉冲 unit，已按运动方向排序
};

/**
 * 直线插补 + 位置比较触发任务参数。
 * 典型流程：配置比较点 →（可选）移动到起始点 → 直线插补到终点，运动过程中触发比较输出。
 */
struct LinearPositionCompareParams {
    std::string axisXName;			// X 轴名称
    std::string axisYName;			// Y 轴名称
    double startX = 0.0;			// 起始 X，工程单位（绝对坐标）
    double startY = 0.0;			// 起始 Y
    double endX = 0.0;				// 终点 X
    double endY = 0.0;				// 终点 Y
    PositionCompareRefAxis compareRefAxis = PositionCompareRefAxis::X;	// 沿 X 或 Y 轴比较
    std::vector<double> comparePositions;	// 比较点绝对坐标（沿 compareRefAxis），工程单位
    PositionCompareChannel channel = PositionCompareChannel::HighSpeed1;
    PositionCompareSource compareSource = PositionCompareSource::EncoderPosition;
    PositionCompareOutputConfig output;
    int crdNo = 0;					// 插补坐标系号
    double speed = 0.0;				// 直线速度，≤0 使用轴默认
    bool moveToStartFirst = true;	// 是否先绝对移动到起始点
    int timeout = 30000;			// 等待运动完成超时，单位：ms
};

/**
 * 单轴定长运动 + 位置比较触发任务参数（工程单位）。
 * 等价于 SetupPositionCompare + AxisMove，与 AxisLinearWithPositionCompare 同族接口。
 */
struct AxisMoveWithPositionCompareParams {
    double startPos = 0.0;			// 起始位置，工程单位（绝对坐标）
    double endPos = 0.0;				// 终点位置
    std::vector<double> comparePositions;	// 比较点绝对坐标，工程单位
    PositionCompareChannel channel = PositionCompareChannel::HighSpeed1;
    PositionCompareSource compareSource = PositionCompareSource::EncoderPosition;
    PositionCompareOutputConfig output;
    double speed = 0.0;				// ≤0 使用轴默认 maxVel
    bool moveToStartFirst = true;	// 是否先绝对移动到起始点
    int timeout = 30000;
};

/** 禁用位置比较参数 */
struct PositionCompareDisableParams {
    std::string compareAxisName;	// 参与比较的轴名称
    PositionCompareChannel channel = PositionCompareChannel::HighSpeed1;
};

// 连接设备(运动板卡)参数结构体
struct DeviceConnectOpts {
    int m_connectNO;                 // 连接号: 指定链接号（0-7）,默认值 0
    int m_connectType;               // 连接类型: 1-串口,2-网口,默认 2
    std::string m_configfile;		 // 配置文件路径
    std::string m_connectString;     // 连接字符串: 对应与控制器的IP地址("127.0.0.1") 或 相应的COM口("COM8")
    uint32_t m_baud;                 // 串口通信波特率(默认:115200)
    int m_parity;                    // 串口通信校验方式(0:none, 1:odd, 2:even)
    int m_dataBit;                   // 串口通信数据位(默认:8)
    int m_stopBit;                   // 串口通信停止位(1:停止位1, 2:停止位2)

    DeviceConnectOpts() {
        m_connectNO = 0;
        m_connectType = 2;
        m_configfile = "";
        m_connectString = "127.0.0.1";
        m_baud = 115200;
        m_parity = 0;
        m_dataBit = 8;
        m_stopBit = 1;
    }

    void reflectFromJson(const json& j) {
        getFromJson(j, "connectNO", m_connectNO);
        getFromJson(j, "connectType", m_connectType);
        getFromJson(j, "configfile", m_configfile);
        getFromJson(j, "connectString", m_connectString);
        getFromJson(j, "baud", m_baud);
        getFromJson(j, "parity", m_parity);
        getFromJson(j, "dataBit", m_dataBit);
        getFromJson(j, "stopBit", m_stopBit);
    }
    void reflectToJson(json& j) const {
        j["connectNO"] = m_connectNO;
        j["connectType"] = m_connectType;
        j["configfile"] = m_configfile;
        j["connectString"] = m_connectString;
        j["baud"] = m_baud;
        j["parity"] = m_parity;
        j["dataBit"] = m_dataBit;
        j["stopBit"] = m_stopBit;
    }

};

// SDK 返回值：能力不支持（与历史 -3 约定一致）
inline constexpr long kSdkNotSupported = -3;

} // namespace motion

} // end of namespace device

} // end of namespace common

#endif // COMMON_CORE_DEVICE_MOTION_CONTROL_MOTION_TYPES_H
