#include "algo_ci_data_convert.h"

#include <ci_define.h>

#include <algorithm>
#include <cstring>
#include <utility>

namespace vision_algo::cisdk_convert {
namespace {

template <typename Dst, typename Src>
Dst enumCast(Src value)
{
    return static_cast<Dst>(static_cast<int>(value));
}

void copyToFixedBuffer(char *dst, std::size_t cap, const std::string &src)
{
    if (!dst || cap == 0)
        return;
    std::strncpy(dst, src.c_str(), cap - 1);
    dst[cap - 1] = '\0';
}

void releaseAlgoShotChildren(AlgoShot &shot)
{
    delete shot.part;
    delete shot.panel;
    delete shot.layer;
    delete shot.pattern;
    delete shot.color;
    delete shot.device;
    delete shot.capture;
    shot.part    = nullptr;
    shot.panel   = nullptr;
    shot.layer   = nullptr;
    shot.pattern = nullptr;
    shot.color   = nullptr;
    shot.device  = nullptr;
    shot.capture = nullptr;
}

void releaseAlgoLedMeterChildren(AlgoLedMeter &meter)
{
    delete meter.part;
    delete meter.panel;
    delete meter.layer;
    delete meter.capture;
    meter.part    = nullptr;
    meter.panel   = nullptr;
    meter.layer   = nullptr;
    meter.capture = nullptr;
}

void releaseCisdkShotChildren(::AlgoShot &shot)
{
    delete shot.part;
    delete shot.panel;
    delete shot.layer;
    delete shot.pattern;
    delete shot.color;
    delete shot.device;
    delete shot.capture;
    shot.part    = nullptr;
    shot.panel   = nullptr;
    shot.layer   = nullptr;
    shot.pattern = nullptr;
    shot.color   = nullptr;
    shot.device  = nullptr;
    shot.capture = nullptr;
}

void releaseCisdkLedMeterChildren(::AlgoLedMeter &meter)
{
    delete meter.part;
    delete meter.panel;
    delete meter.layer;
    delete meter.capture;
    meter.part    = nullptr;
    meter.panel   = nullptr;
    meter.layer   = nullptr;
    meter.capture = nullptr;
}

} // namespace

::AlgoSize toCisdkSize(const AlgoSize &size)
{
    return ::AlgoSize{size.width, size.height};
}

AlgoSize fromCisdkSize(const ::AlgoSize &size)
{
    return AlgoSize{size.width, size.height};
}

::AlgoSize2f toCisdkSize2f(const AlgoSize2f &size)
{
    return ::AlgoSize2f{size.width, size.height};
}

AlgoSize2f fromCisdkSize2f(const ::AlgoSize2f &size)
{
    return AlgoSize2f{size.width, size.height};
}

::AlgoPoint toCisdkPoint(const AlgoPoint &point)
{
    return ::AlgoPoint{point.x, point.y};
}

AlgoPoint fromCisdkPoint(const ::AlgoPoint &point)
{
    return AlgoPoint{point.x, point.y};
}

::AlgoPoint2f toCisdkPoint2f(const AlgoPoint2f &point)
{
    return ::AlgoPoint2f{point.x, point.y};
}

AlgoPoint2f fromCisdkPoint2f(const ::AlgoPoint2f &point)
{
    return AlgoPoint2f{point.x, point.y};
}

::AlgoRect toCisdkRect(const AlgoRect &rect)
{
    return ::AlgoRect{rect.x, rect.y, rect.width, rect.height};
}

AlgoRect fromCisdkRect(const ::AlgoRect &rect)
{
    return AlgoRect{rect.x, rect.y, rect.width, rect.height};
}

::AlgoRect2f toCisdkRect2f(const AlgoRect2f &rect)
{
    return ::AlgoRect2f{rect.x, rect.y, rect.width, rect.height};
}

AlgoRect2f fromCisdkRect2f(const ::AlgoRect2f &rect)
{
    return AlgoRect2f{rect.x, rect.y, rect.width, rect.height};
}

AlgoCornerPoint cornerPointsFromRect(const AlgoRect &rect)
{
    AlgoCornerPoint corner{};
    corner.p1 = {static_cast<float>(rect.x), static_cast<float>(rect.y)};
    corner.p2 = {static_cast<float>(rect.x + rect.width), static_cast<float>(rect.y)};
    corner.p3 = {static_cast<float>(rect.x + rect.width), static_cast<float>(rect.y + rect.height)};
    corner.p4 = {static_cast<float>(rect.x), static_cast<float>(rect.y + rect.height)};
    return corner;
}

::AlgoMat toCisdkMat(const AlgoMat &mat)
{
    ::AlgoMat dst{};
    dst.rows  = mat.rows;
    dst.cols  = mat.cols;
    dst.type  = enumCast<::AlgoMatType>(mat.type);
    dst.len   = mat.len;
    dst.data  = mat.data;
    return dst;
}

AlgoMat fromCisdkMat(const ::AlgoMat &mat)
{
    AlgoMat dst;
    dst.rows  = mat.rows;
    dst.cols  = mat.cols;
    dst.type  = enumCast<AlgoMatType>(mat.type);
    dst.len   = mat.len;
    dst.data  = mat.data;
    return dst;
}

TriLab toCisdk(const AlgoTriLab &v)
{
    return TriLab{v.L, v.a, v.b};
}

AlgoTriLab fromCisdk(const TriLab &v)
{
    return AlgoTriLab{v.L, v.a, v.b};
}

TriLch toCisdk(const AlgoTriLch &v)
{
    return TriLch{v.L, v.c, v.h};
}

AlgoTriLch fromCisdk(const TriLch &v)
{
    return AlgoTriLch{v.L, v.c, v.h};
}

TriLxy toCisdk(const AlgoTriLxy &v)
{
    return TriLxy{v.L, v.x, v.y};
}

AlgoTriLxy fromCisdk(const TriLxy &v)
{
    return AlgoTriLxy{v.L, v.x, v.y};
}

TriXYZ toCisdk(const AlgoTriXYZ &v)
{
    return TriXYZ{v.X, v.Y, v.Z};
}

AlgoTriXYZ fromCisdk(const TriXYZ &v)
{
    return AlgoTriXYZ{v.X, v.Y, v.Z};
}

TriRGB toCisdk(const AlgoTriRGB &v)
{
    return TriRGB{v.R, v.G, v.B};
}

AlgoTriRGB fromCisdk(const TriRGB &v)
{
    return AlgoTriRGB{v.R, v.G, v.B};
}

RGBW toCisdk(const AlgoRGBW &v)
{
    return RGBW{v.R, v.G, v.B, v.W};
}

AlgoRGBW fromCisdk(const RGBW &v)
{
    return AlgoRGBW{v.R, v.G, v.B, v.W};
}

RGBLxy toCisdk(const AlgoRGBLxy &v)
{
    return RGBLxy{toCisdk(v.R), toCisdk(v.G), toCisdk(v.B)};
}

AlgoRGBLxy fromCisdk(const RGBLxy &v)
{
    return AlgoRGBLxy{fromCisdk(v.R), fromCisdk(v.G), fromCisdk(v.B)};
}

RGBWLxy toCisdk(const AlgoRGBWLxy &v)
{
    return RGBWLxy{toCisdk(v.R), toCisdk(v.G), toCisdk(v.B), toCisdk(v.W)};
}

AlgoRGBWLxy fromCisdk(const RGBWLxy &v)
{
    return AlgoRGBWLxy{fromCisdk(v.R), fromCisdk(v.G), fromCisdk(v.B), fromCisdk(v.W)};
}

RGBWXYZ toCisdk(const AlgoRGBWXYZ &v)
{
    return RGBWXYZ{toCisdk(v.R), toCisdk(v.G), toCisdk(v.B), toCisdk(v.W)};
}

AlgoRGBWXYZ fromCisdk(const RGBWXYZ &v)
{
    return AlgoRGBWXYZ{fromCisdk(v.R), fromCisdk(v.G), fromCisdk(v.B), fromCisdk(v.W)};
}

RGBXYZ toCisdk(const AlgoRGBXYZ &v)
{
    return RGBXYZ{toCisdk(v.R), toCisdk(v.G), toCisdk(v.B)};
}

AlgoRGBXYZ fromCisdk(const RGBXYZ &v)
{
    return AlgoRGBXYZ{fromCisdk(v.R), fromCisdk(v.G), fromCisdk(v.B)};
}

Device toCisdk(const AlgoDevice &device)
{
    Device dst{};
    dst.cam_type  = enumCast<CameraType>(device.cam_type);
    dst.lens_type = enumCast<LensType>(device.lens_type);
    copyToFixedBuffer(dst.id, ALGO_DEVICE_STR_LEN, device.device_id);
    copyToFixedBuffer(dst.lens_id, ALGO_DEVICE_STR_LEN, device.lens_id);
    return dst;
}

AlgoDevice fromCisdk(const Device &device)
{
    AlgoDevice dst;
    dst.device_id = device.id;
    dst.lens_id   = device.lens_id;
    dst.cam_type  = enumCast<AlgoCameraType>(device.cam_type);
    dst.lens_type = enumCast<AlgoLensType>(device.lens_type);
    return dst;
}

Capture toCisdk(const AlgoCapture &capture)
{
    Capture dst{};
    dst.exposure   = capture.exposure;
    dst.gain       = capture.gain;
    dst.aperture   = capture.aperture;
    dst.microfocus = capture.microfocus;
    dst.filter     = enumCast<CamFilter>(capture.filter);
    return dst;
}

AlgoCapture fromCisdk(const Capture &capture)
{
    AlgoCapture dst;
    dst.exposure   = capture.exposure;
    dst.gain       = capture.gain;
    dst.aperture   = capture.aperture;
    dst.microfocus = capture.microfocus;
    dst.filter     = enumCast<AlgoCamFilter>(capture.filter);
    return dst;
}

Layer toCisdk(const AlgoLayer &layer)
{
    return Layer{layer.index, layer.linear_norms, layer.value};
}

AlgoLayer fromCisdk(const Layer &layer)
{
    return AlgoLayer{layer.index, layer.linear_norms, layer.value};
}

LedPanel toCisdk(const AlgoLedPanel &panel)
{
    LedPanel dst{};
    dst.type          = enumCast<LedPanelType>(panel.type);
    dst.layout        = enumCast<LedLayout>(panel.layout);
    dst.subpixel_type = enumCast<LedSubpixelType>(panel.subpixel_type);
    dst.wh            = toCisdkSize(panel.wh);
    dst.pixel_pitch   = panel.pixel_pitch;
    return dst;
}

AlgoLedPanel fromCisdk(const LedPanel &panel)
{
    AlgoLedPanel dst;
    dst.type          = enumCast<AlgoLedPanelType>(panel.type);
    dst.layout        = enumCast<AlgoLedLayout>(panel.layout);
    dst.subpixel_type = enumCast<AlgoLedSubpixelType>(panel.subpixel_type);
    dst.wh            = fromCisdkSize(panel.wh);
    dst.pixel_pitch   = panel.pixel_pitch;
    return dst;
}

Pattern toCisdk(const AlgoPattern &pattern)
{
    Pattern dst{};
    dst.type           = enumCast<PatternType>(pattern.type);
    dst.interval       = toCisdkSize(pattern.interval);
    dst.loc            = toCisdkPoint(pattern.loc);
    dst.lighted_led_wh = toCisdkSize(pattern.lighted_led_wh);
    return dst;
}

AlgoPattern fromCisdk(const Pattern &pattern)
{
    AlgoPattern dst;
    dst.type           = enumCast<AlgoPatternType>(pattern.type);
    dst.interval       = fromCisdkSize(pattern.interval);
    dst.loc            = fromCisdkPoint(pattern.loc);
    dst.lighted_led_wh = fromCisdkSize(pattern.lighted_led_wh);
    return dst;
}

Partition toCisdk(const AlgoPartition &partition)
{
    Partition dst{};
    dst.xywh      = toCisdkRect(partition.xywh);
    dst.screen_wh = toCisdkSize(partition.screen_wh);
    return dst;
}

AlgoPartition fromCisdk(const Partition &partition)
{
    AlgoPartition dst;
    dst.xywh      = fromCisdkRect(partition.xywh);
    dst.screen_wh = fromCisdkSize(partition.screen_wh);
    return dst;
}

void releaseAlgoShot(AlgoShot &shot)
{
    releaseAlgoShotChildren(shot);
}

AlgoShot fromCisdkShot(const ::AlgoShot &shot)
{
    AlgoShot dst;
    dst.type  = enumCast<AlgoCIEType>(shot.type);
    dst.rect  = fromCisdkRect(shot.rect);
    dst.image = fromCisdkMat(shot.image);
    if (shot.part)
        dst.part = new AlgoPartition(fromCisdk(*shot.part));
    if (shot.panel)
        dst.panel = new AlgoLedPanel(fromCisdk(*shot.panel));
    if (shot.layer)
        dst.layer = new AlgoLayer(fromCisdk(*shot.layer));
    if (shot.pattern)
        dst.pattern = new AlgoPattern(fromCisdk(*shot.pattern));
    if (shot.color)
        dst.color = new AlgoColor(enumCast<AlgoColor>(*shot.color));
    if (shot.device)
        dst.device = new AlgoDevice(fromCisdk(*shot.device));
    if (shot.capture)
        dst.capture = new AlgoCapture(fromCisdk(*shot.capture));
    return dst;
}

::AlgoShot toCisdkShot(const AlgoShot &shot)
{
    ::AlgoShot dst{};
    dst.type  = enumCast<CIEType>(shot.type);
    dst.rect  = toCisdkRect(shot.rect);
    dst.image = toCisdkMat(shot.image);
    if (shot.part)
        dst.part = new Partition(toCisdk(*shot.part));
    if (shot.panel)
        dst.panel = new LedPanel(toCisdk(*shot.panel));
    if (shot.layer)
        dst.layer = new Layer(toCisdk(*shot.layer));
    if (shot.pattern)
        dst.pattern = new Pattern(toCisdk(*shot.pattern));
    if (shot.color)
        dst.color = new ::AlgoColor(enumCast<::AlgoColor>(*shot.color));
    if (shot.device)
        dst.device = new Device(toCisdk(*shot.device));
    if (shot.capture)
        dst.capture = new Capture(toCisdk(*shot.capture));
    return dst;
}

void releaseCisdkShot(::AlgoShot &shot)
{
    releaseCisdkShotChildren(shot);
}

void releaseAlgoLedMeter(AlgoLedMeter &meter)
{
    releaseAlgoLedMeterChildren(meter);
}

AlgoLedMeter fromCisdkLedMeter(const ::AlgoLedMeter &meter)
{
    AlgoLedMeter dst;
    dst.color  = enumCast<AlgoColor>(meter.color);
    dst.type   = enumCast<AlgoCIEType>(meter.type);
    dst.state  = fromCisdkMat(meter.state);
    dst.pos    = fromCisdkMat(meter.pos);
    dst.chroma = fromCisdkMat(meter.chroma);
    if (meter.part)
        dst.part = new AlgoPartition(fromCisdk(*meter.part));
    if (meter.panel)
        dst.panel = new AlgoLedPanel(fromCisdk(*meter.panel));
    if (meter.layer)
        dst.layer = new AlgoLayer(fromCisdk(*meter.layer));
    if (meter.capture)
        dst.capture = new AlgoCapture(fromCisdk(*meter.capture));
    return dst;
}

::AlgoLedMeter toCisdkLedMeter(const AlgoLedMeter &meter)
{
    ::AlgoLedMeter dst{};
    dst.color  = enumCast<::AlgoColor>(meter.color);
    dst.type   = enumCast<CIEType>(meter.type);
    dst.state  = toCisdkMat(meter.state);
    dst.pos    = toCisdkMat(meter.pos);
    dst.chroma = toCisdkMat(meter.chroma);
    if (meter.part)
        dst.part = new Partition(toCisdk(*meter.part));
    if (meter.panel)
        dst.panel = new LedPanel(toCisdk(*meter.panel));
    if (meter.layer)
        dst.layer = new Layer(toCisdk(*meter.layer));
    if (meter.capture)
        dst.capture = new Capture(toCisdk(*meter.capture));
    return dst;
}

void releaseCisdkLedMeter(::AlgoLedMeter &meter)
{
    releaseCisdkLedMeterChildren(meter);
}

::AlgoLedMeterMergeInfo toCisdk(const AlgoLedMeterMergeInfo &info)
{
    ::AlgoLedMeterMergeInfo dst{};
    dst.motion_path            = enumCast<::AlgoLedMeterMotionPathType>(info.motion_path);
    dst.horizontal_shot_num    = info.horizontal_shot_num;
    dst.vertical_shot_num      = info.vertical_shot_num;
    dst.horizontal_overlap_num = info.horizontal_overlap_num;
    dst.vertical_overlap_num   = info.vertical_overlap_num;
    dst.led_wh                 = toCisdkSize(info.led_wh);
    return dst;
}

AlgoLedMeterMergeInfo fromCisdk(const ::AlgoLedMeterMergeInfo &info)
{
    AlgoLedMeterMergeInfo dst;
    dst.motion_path            = enumCast<AlgoLedMeterMotionPathType>(info.motion_path);
    dst.horizontal_shot_num    = info.horizontal_shot_num;
    dst.vertical_shot_num      = info.vertical_shot_num;
    dst.horizontal_overlap_num = info.horizontal_overlap_num;
    dst.vertical_overlap_num   = info.vertical_overlap_num;
    dst.led_wh                 = fromCisdkSize(info.led_wh);
    return dst;
}

#ifdef VISION_ALGO_HAS_CHIP

ChipDetectRGBType toCisdkRgbType(ChipDetectRgbType type)
{
    return static_cast<ChipDetectRGBType>(static_cast<int>(type));
}

ChipDetectRGBAlignType toCisdkRgbAlignType(ChipDetectRgbAlignType type)
{
    return static_cast<ChipDetectRGBAlignType>(static_cast<int>(type));
}

ChipDetectThresh toCisdkChipThreshold(const ChipDetectThreshold &thresh)
{
    ChipDetectThresh dst{};
    dst.in_match_thresh = thresh.matchThresh;
    dst.in_low_thresh   = thresh.lowThresh;
    dst.in_high_thresh  = thresh.highThresh;
    dst.in_area_thresh  = thresh.areaThresh;
    return dst;
}

#endif // VISION_ALGO_HAS_CHIP

#ifdef VISION_ALGO_HAS_DIE_BONDING

namespace {

ChipChannelType mapChipType(DieBondingChipType type)
{
    switch (type) {
    case CHIP_R:
        return ChipChannelType::R;
    case CHIP_G:
        return ChipChannelType::G;
    case CHIP_B:
        return ChipChannelType::B;
    default:
        return ChipChannelType::None;
    }
}

AlgoRect2f mapRect(const ::AlgoRect &rect)
{
    return AlgoRect2f{static_cast<float>(rect.x), static_cast<float>(rect.y),
                      static_cast<float>(rect.width), static_cast<float>(rect.height)};
}

} // namespace

ChipDetectInfo fromCisdkChip(const DieBondingChipInfo &src)
{
    ChipDetectInfo dst;
    dst.chipType             = mapChipType(src.chip_type);
    dst.chipActualType       = mapChipType(src.chip_actual_type);
    dst.polarType            = static_cast<int>(src.polar_type);
    dst.solderCoordinate     = fromCisdkPoint2f(src.solder_coordinate);
    dst.chipCoordinate       = fromCisdkPoint2f(src.chip_coordinate);
    dst.solderChipDeflection = fromCisdkPoint2f(src.solder_chip_deflection);
    dst.chipAngle            = src.chip_angle;
    dst.chipArea             = src.chip_area;
    dst.chipWidth            = src.chip_width;
    dst.chipHeight           = src.chip_height;
    dst.solderWidth          = src.solder_width;
    dst.solderHeight         = src.solder_height;
    dst.chipDefect           = src.chip_defect;
    dst.chipOffsetRow        = src.chip_offset_row;
    dst.chipOffsetCol        = src.chip_offset_col;
    dst.chipOffsetValueRow   = src.chip_offset_value_row;
    dst.chipOffsetValueCol   = src.chip_offset_value_col;
    dst.chipImageRect        = mapRect(src.chip_image_rect);
    return dst;
}

ChipGroupDetectInfo fromCisdkGroup(const DieBondingChipGroupInfo &src)
{
    ChipGroupDetectInfo dst;
    dst.row           = src.row;
    dst.col           = src.col;
    dst.chipGroupRect = mapRect(src.chip_group_rect);
    dst.coordinate    = fromCisdkPoint2f(src.coordinate);
    for (int i = 0; i < 3; ++i)
        dst.chipRgbInfo[i] = fromCisdkChip(src.chip_rgb_info[i]);
    dst.imageIndex = src.image_index;
    return dst;
}

std::vector<ChipGroupDetectInfo> fromCisdkGroups(const std::vector<DieBondingChipGroupInfo> &src)
{
    std::vector<ChipGroupDetectInfo> dst;
    dst.reserve(src.size());
    for (const auto &group : src)
        dst.push_back(fromCisdkGroup(group));
    return dst;
}

DieBondingChipThreshold toCisdkThreshold(const ChipThreshold &src)
{
    DieBondingChipThreshold dst{};
    dst.chip_match_threshold   = src.chipMatchThreshold;
    dst.chip_segment_threshold = src.chipSegmentThreshold;
    dst.polar_match_threshold  = src.polarMatchThreshold;
    return dst;
}

#endif // VISION_ALGO_HAS_DIE_BONDING

} // namespace vision_algo::cisdk_convert

namespace vision_algo {

AlgoShot::~AlgoShot()
{
    cisdk_convert::releaseAlgoShot(*this);
}

AlgoShot::AlgoShot(AlgoShot &&other) noexcept
    : part(other.part)
    , panel(other.panel)
    , layer(other.layer)
    , pattern(other.pattern)
    , color(other.color)
    , device(other.device)
    , capture(other.capture)
    , rect(other.rect)
    , type(other.type)
    , image(other.image)
{
    other.part    = nullptr;
    other.panel   = nullptr;
    other.layer   = nullptr;
    other.pattern = nullptr;
    other.color   = nullptr;
    other.device  = nullptr;
    other.capture = nullptr;
}

AlgoShot &AlgoShot::operator=(AlgoShot &&other) noexcept
{
    if (this != &other) {
        cisdk_convert::releaseAlgoShot(*this);
        part    = other.part;
        panel   = other.panel;
        layer   = other.layer;
        pattern = other.pattern;
        color   = other.color;
        device  = other.device;
        capture = other.capture;
        rect    = other.rect;
        type    = other.type;
        image   = other.image;

        other.part    = nullptr;
        other.panel   = nullptr;
        other.layer   = nullptr;
        other.pattern = nullptr;
        other.color   = nullptr;
        other.device  = nullptr;
        other.capture = nullptr;
    }
    return *this;
}

AlgoLedMeter::~AlgoLedMeter()
{
    cisdk_convert::releaseAlgoLedMeter(*this);
}

AlgoLedMeter::AlgoLedMeter(AlgoLedMeter &&other) noexcept
    : part(other.part)
    , panel(other.panel)
    , layer(other.layer)
    , capture(other.capture)
    , color(other.color)
    , type(other.type)
    , state(other.state)
    , pos(other.pos)
    , chroma(other.chroma)
{
    other.part    = nullptr;
    other.panel   = nullptr;
    other.layer   = nullptr;
    other.capture = nullptr;
}

AlgoLedMeter &AlgoLedMeter::operator=(AlgoLedMeter &&other) noexcept
{
    if (this != &other) {
        cisdk_convert::releaseAlgoLedMeter(*this);
        part    = other.part;
        panel   = other.panel;
        layer   = other.layer;
        capture = other.capture;
        color   = other.color;
        type    = other.type;
        state   = other.state;
        pos     = other.pos;
        chroma  = other.chroma;

        other.part    = nullptr;
        other.panel   = nullptr;
        other.layer   = nullptr;
        other.capture = nullptr;
    }
    return *this;
}

} // namespace vision_algo
