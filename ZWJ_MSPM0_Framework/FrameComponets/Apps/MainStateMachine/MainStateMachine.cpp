#include "MainStateMachine.hpp"
#include "Follow.hpp"
#include "Navigation.hpp"
#include "Overtake.hpp"
#include "SpeedMixer.hpp"
#include "System.hpp"
#include "Track.hpp"
#include "TurnAround.hpp"
#include "motor_at8236.hpp"

StateGraph MainStateMachine::main_graph("MainGraph");
StateBlock *MainStateMachine::st_idle = nullptr;
StateBlock *MainStateMachine::st_track = nullptr;
StateBlock *MainStateMachine::st_follow = nullptr;
StateBlock *MainStateMachine::st_overtake = nullptr;
StateBlock *MainStateMachine::st_turn_around = nullptr;
StateBlock *MainStateMachine::st_navigation = nullptr;
StateBlock *MainStateMachine::st_finish = nullptr;

bool MainStateMachine::cond_start = false;
bool MainStateMachine::cond_has_car = false;
bool MainStateMachine::cond_no_car = true; // 初始无车
bool MainStateMachine::cond_dashed_line = false;
bool MainStateMachine::cond_overtake_done = false;
bool MainStateMachine::cond_finish_line = false;
bool MainStateMachine::cond_turn_done = false;
bool MainStateMachine::cond_return_start = false;
bool MainStateMachine::cond_nav_start = false;
bool MainStateMachine::cond_nav_done = false;

// 掉头完成后的返回标志（静态全局）
static bool has_turned = false;

// ========== 公共接口实现 ==========
void MainStateMachine::Init() {
    RegisterAllApps();      // 1. 先注册所有 App
    InitStateBlocks();      // 2. 再初始化状态块
    InitStateTransitions(); // 3. 最后建立状态转换链接
}

void MainStateMachine::Start() {
    StateCore &core = StateCore::GetInstance();
    core.RegistGraph(main_graph); // 注册主状态图
    core.Enable(0);               // 从第 0 个状态图开始执行
}

const char *MainStateMachine::GetCurrentStateName() {
    StateCore &core = StateCore::GetInstance();
    return core.GetCurState().name;
}

// ========== 内部初始化函数实现 ==========
void MainStateMachine::RegisterAllApps() {
    // 按顺序注册所有 App 到 System
    System.RegistApp(track_app);
    System.RegistApp(follow_app);
    System.RegistApp(overtake_app);
    System.RegistApp(turn_around_app);
    System.RegistApp(navigation_app);
}

void MainStateMachine::InitStateBlocks() {
    // 1. 添加所有状态块到状态图
    st_idle = &main_graph.AddState("Idle");
    st_track = &main_graph.AddState("Track");
    st_follow = &main_graph.AddState("Follow");
    st_overtake = &main_graph.AddState("Overtake");
    st_turn_around = &main_graph.AddState("TurnAround");
    st_navigation = &main_graph.AddState("Navigation");
    st_finish = &main_graph.AddState("Finish");

    // 2. 绑定每个状态的执行函数
    st_idle->StateAction = ActionIdle;
    st_track->StateAction = ActionTrack;
    st_follow->StateAction = ActionFollow;
    st_overtake->StateAction = ActionOvertake;
    st_turn_around->StateAction = ActionTurnAround;
    st_navigation->StateAction = ActionNavigation;
    st_finish->StateAction = ActionFinish;
}

void MainStateMachine::InitStateTransitions() {
    // Idle → 巡线
    st_idle->LinkTo(&cond_start, *st_track);

    // Track 的转换：优先检查返回起点 (Finish)，再检查掉头，最后检查跟车
    st_track->LinkTo(&cond_return_start, *st_finish);     // ① 最高优先级
    st_track->LinkTo(&cond_finish_line, *st_turn_around); // ② 终点掉头
    st_track->LinkTo(&cond_has_car, *st_follow);          // ③ 有车跟车

    // Follow 的转换
    st_follow->LinkTo(&cond_no_car, *st_track);
    st_follow->LinkTo(&cond_finish_line, *st_turn_around);

    // 掉头完成 → 巡线
    st_turn_around->LinkTo(&cond_turn_done, *st_track);

    // 超车（暂屏蔽）
    // st_track->LinkTo(&cond_dashed_line, *st_overtake);
    // st_follow->LinkTo(&cond_dashed_line, *st_overtake);
    // st_overtake->LinkTo(&cond_overtake_done, *st_follow);

    // 第一阶段完成 → 导航
    st_finish->LinkTo(&cond_nav_start, *st_navigation);
}

// ========== 状态动作函数实现（核心联动逻辑） ==========
/**
 * @brief 空闲状态动作
 * @note 所有 App 禁用，等待开始信号
 */
void MainStateMachine::ActionIdle(StateCore *core) {
    // 1. 禁用所有 App
    track_app.SetEnable(false);
    follow_app.SetEnable(false);
    overtake_app.SetEnable(false);
    turn_around_app.SetEnable(false);
    navigation_app.SetEnable(false);

    // 2. 清除 SpeedMixer 所有速度设置
    speed_mixer.ClearAll();

    // 3. 立即开始并重置掉头标志
    cond_start = true;
    has_turned = false;
}

/**
 * @brief 纯巡线状态动作
 * @note 只启用 Track，禁用其他，更新巡线相关条件
 */
void MainStateMachine::ActionTrack(StateCore *core) {
    // 1. 启用/禁用对应 App
    track_app.SetEnable(true);
    overtake_app.SetEnable(false);
    turn_around_app.SetEnable(false);
    navigation_app.SetEnable(false);

    // 必须启用 Follow 以便其 Update 被调用，从而更新超声波距离
    follow_app.SetEnable(true);
    follow_app.SetOutputEnable(false); // 禁止输出速度偏移

    // 2. 清除不需要的 SpeedMixer 来源
    speed_mixer.ClearSource(SpeedMixer::Source::FOLLOW);
    speed_mixer.ClearSource(SpeedMixer::Source::OVERTAKE);
    speed_mixer.ClearSource(SpeedMixer::Source::NAVIGATION);
    speed_mixer.ClearSource(SpeedMixer::Source::TURN_AROUND);

    // 3. 更新状态转换条件
    cond_has_car = (follow_app.real_dist > 0 && follow_app.real_dist < 60.0f);
    cond_no_car = !cond_has_car;
    cond_dashed_line = track_app.is_dashed_line;
    cond_finish_line = track_app.is_finish_line;

    // 返回起点条件：已经掉头过，且再次检测到十字线
    cond_return_start = (has_turned && track_app.is_finish_line);
}

/**
 * @brief 巡线+跟车状态动作
 * @note 同时启用 Track 和 Follow，更新跟车相关条件
 */
void MainStateMachine::ActionFollow(StateCore *core) {
    // 1. 启用/禁用对应 App
    track_app.SetEnable(true);
    overtake_app.SetEnable(false);
    turn_around_app.SetEnable(false);
    navigation_app.SetEnable(false);

    follow_app.SetEnable(true);
    follow_app.SetOutputEnable(true); // 允许输出跟车速度偏移

    // 2. 清除不需要的 SpeedMixer 来源
    speed_mixer.ClearSource(SpeedMixer::Source::OVERTAKE);
    speed_mixer.ClearSource(SpeedMixer::Source::NAVIGATION);
    speed_mixer.ClearSource(SpeedMixer::Source::TURN_AROUND);

    // 3. 更新状态转换条件
    cond_has_car = (follow_app.real_dist > 0 && follow_app.real_dist < 60.0f);
    cond_no_car = !cond_has_car;
    cond_dashed_line = track_app.is_dashed_line;
    cond_finish_line = track_app.is_finish_line;
}

/**
 * @brief 超车状态动作
 * @note 只启用 Overtake，启动超车流程
 */
void MainStateMachine::ActionOvertake(StateCore *core) {
    // 1. 启用/禁用对应 App
    track_app.SetEnable(false);
    follow_app.SetEnable(false);
    overtake_app.SetEnable(true);
    turn_around_app.SetEnable(false);
    navigation_app.SetEnable(false);

    // 2. 清除不需要的 SpeedMixer 来源
    speed_mixer.ClearSource(SpeedMixer::Source::TRACK);
    speed_mixer.ClearSource(SpeedMixer::Source::FOLLOW);
    speed_mixer.ClearSource(SpeedMixer::Source::NAVIGATION);
    speed_mixer.ClearSource(SpeedMixer::Source::TURN_AROUND);

    // 3. 启动超车（仅在进入状态时执行一次）
    static bool first_enter = true;
    if (first_enter) {
        overtake_app.StartOvertake();
        first_enter = false;
    }

    // 4. 更新完成标志
    cond_overtake_done = overtake_app.is_complete;

    // 5. 退出状态时重置标志
    if (cond_overtake_done) {
        first_enter = true;
        cond_dashed_line = false; // 清除虚线标志，避免重复触发
    }
}

/**
 * @brief 掉头状态动作
 * @note 所有 App 禁用，直接执行掉头逻辑
 */
void MainStateMachine::ActionTurnAround(StateCore *core) {
    // 跟踪继续运行（用于更新灰度传感器），但清除巡线速度源
    track_app.SetEnable(true);
    follow_app.SetEnable(false);
    overtake_app.SetEnable(false);
    turn_around_app.SetEnable(true);
    navigation_app.SetEnable(false);

    speed_mixer.ClearSource(SpeedMixer::Source::TRACK);
    speed_mixer.ClearSource(SpeedMixer::Source::FOLLOW);
    speed_mixer.ClearSource(SpeedMixer::Source::OVERTAKE);
    speed_mixer.ClearSource(SpeedMixer::Source::NAVIGATION);
    // 不清除 TURN_AROUND，让它生效

    cond_turn_done = turn_around_app.is_complete;
    if (cond_turn_done) {
        has_turned = true;
    }
}

/**
 * @brief 导航状态动作
 * @note 只启用 Navigation
 */
void MainStateMachine::ActionNavigation(StateCore *core) {
    cond_return_start = false;

    // 1. 启用/禁用对应 App
    track_app.SetEnable(false);
    follow_app.SetEnable(false);
    overtake_app.SetEnable(false);
    turn_around_app.SetEnable(false);
    navigation_app.SetEnable(true);

    // 2. 清除所有其他速度源，Navigation 会自行设置速度
    speed_mixer.ClearSource(SpeedMixer::Source::TRACK);
    speed_mixer.ClearSource(SpeedMixer::Source::FOLLOW);
    speed_mixer.ClearSource(SpeedMixer::Source::OVERTAKE);
    speed_mixer.ClearSource(SpeedMixer::Source::TURN_AROUND);

    // 3. 更新完成标志
    cond_nav_done = navigation_app.is_complete;
}

/**
 * @brief 结束状态动作
 * @note 所有 App 禁用，电机停止
 */
void MainStateMachine::ActionFinish(StateCore *core) {
    DL_GPIO_writePins(LED_PORT, LED_LED_PIN_PIN);
    // 1. 禁用所有 App
    track_app.SetEnable(false);
    follow_app.SetEnable(false);
    overtake_app.SetEnable(false);
    turn_around_app.SetEnable(false);
    navigation_app.SetEnable(true); // 需要检测导航是否收到消息

    // 2. 清除 SpeedMixer 所有速度设置
    speed_mixer.ClearAll();

    // 3. 检测是否进入导航
    cond_nav_start = navigation_app.is_vaild;
}