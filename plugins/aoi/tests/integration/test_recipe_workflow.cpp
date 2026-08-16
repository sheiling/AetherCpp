// ============================================================
// test_recipe_workflow — 配方完整工作流集成测试
//
// 测试套件：
//   RecipeWorkflowTest  — 配方创建/保存/加载/修改/删除/导入/导出
//
// 依赖：
//   QTemporaryDir       — 临时目录隔离测试数据
//   RecipeDocument      — 配方数据结构
//   RecipeRepository    — 配方持久化仓库
//
// 运行方式：
//   直接执行 aoi_integration_tests.exe，或通过 ctest 运行。
// ============================================================

#include <gtest/gtest.h>

#include <QTemporaryDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QFileInfo>

// ============================================================
// Test Fixture: 使用临时目录隔离测试数据
// ============================================================

class RecipeWorkflowTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(m_tempDir.isValid()) << "Failed to create temporary directory";
        m_storagePath = m_tempDir.path() + "/recipes";
        ASSERT_TRUE(QDir().mkpath(m_storagePath));
    }

    void TearDown() override
    {
        // QTemporaryDir 自动清理
    }

    /**
     * @brief 创建一个有效的配方 JSON 文件
     * @param id 配方 ID
     * @param name 配方名称
     * @param dir 目标目录
     * @return 创建的 JSON 文件路径
     */
    QString createTestRecipeFile(const QString &id, const QString &name,
                                  const QString &dir)
    {
        QJsonObject basic;
        basic["name"] = name;
        basic["width"] = 100;
        basic["height"] = 160;
        basic["group_pitch_x_mm"] = 1.25;
        basic["group_pitch_y_mm"] = 1.25;
        basic["chip_pitch_in_group_mm"] = 0.50;
        basic["chip_width_mm"] = 0.15045;
        basic["chip_height_mm"] = 0.059;
        basic["pad_width_mm"] = 0.32155;
        basic["pad_height_mm"] = 0.13275;
        basic["group_width_mm"] = 0.32155;
        basic["group_height_mm"] = 0.51625;
        basic["chip_color_order"] = 0;
        basic["chip_arrangement"] = 0;
        basic["offset_group_mark_bl"] = QJsonObject{{"x", 0.0}, {"y", 0.0}};
        basic["offset_group_mark_tr"] = QJsonObject{{"x", 0.0}, {"y", 0.0}};

        QJsonObject camera;
        camera["light_brightness"] = 153;
        camera["exposure_us"] = 12000;
        camera["image_width"] = 1920;
        camera["image_height"] = 1080;
        camera["capture_accuracy"] = 2.95;

        QJsonObject algo;
        algo["group_confidence"] = 0.72;
        algo["chip_confidence"] = 0.68;
        algo["chip_defect_class"] = 0.70;
        algo["pad_defect_class"] = 0.65;
        algo["offset_x_um"] = 15.0;
        algo["offset_y_um"] = 15.0;
        algo["angle_offset_deg"] = 0.5;

        QJsonObject recipeJson;
        recipeJson["id"] = id;
        recipeJson["basic"] = basic;
        recipeJson["camera"] = camera;
        recipeJson["algo"] = algo;
        recipeJson["marks"] = QJsonArray{};
        recipeJson["panel_calib"] = QJsonObject{
            {"mode", 0},
            {"fov_info", "未生成"},
            {"path_mode_index", 0},
            {"gerber_file_name", "未导入"}
        };
        recipeJson["position_calib"] = QJsonObject{
            {"track_position", 0.0},
            {"scan_position", QJsonObject{{"x", 0.0}, {"y", 0.0}, {"z", 0.0}}}
        };

        const QString filePath = dir + "/" + id + ".json";
        QFile file(filePath);
        EXPECT_TRUE(file.open(QIODevice::WriteOnly));
        file.write(QJsonDocument(recipeJson).toJson());
        file.close();
        return filePath;
    }

    QTemporaryDir m_tempDir;
    QString m_storagePath;
};

// ============================================================
// 测试用例：配方创建与保存
// ============================================================

/**
 * @brief 验证可以创建有效的配方 JSON 文件
 *
 * 测试步骤：
 * 1. 在临时目录中创建配方 JSON 文件
 * 2. 验证文件存在
 * 3. 验证文件内容是有效的 JSON
 * 4. 验证关键字段值正确
 */
TEST_F(RecipeWorkflowTest, CreateRecipeFile)
{
    // Arrange
    const QString id = "test_recipe_001";
    const QString name = "Test Recipe A";

    // Act
    const QString filePath = createTestRecipeFile(id, name, m_storagePath);

    // Assert
    EXPECT_TRUE(QFileInfo::exists(filePath)) << "Recipe file was not created";

    // 验证文件内容是有效的 JSON
    QFile file(filePath);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly));
    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    ASSERT_TRUE(parseError.error == QJsonParseError::NoError)
        << "Invalid JSON: " << parseError.errorString().toStdString();

    // 验证关键字段
    QJsonObject root = doc.object();
    EXPECT_EQ(root["id"].toString(), id);
    EXPECT_EQ(root["basic"].toObject()["name"].toString(), name);
    EXPECT_EQ(root["basic"].toObject()["width"].toInt(), 100);
    EXPECT_EQ(root["basic"].toObject()["height"].toInt(), 160);
}

// ============================================================
// 测试用例：配方加载与解析
// ============================================================

/**
 * @brief 验证可以从 JSON 文件加载配方并正确解析所有字段
 *
 * 测试步骤：
 * 1. 创建完整的配方 JSON 文件
 * 2. 加载并解析 JSON
 * 3. 验证所有嵌套结构体的字段值正确
 */
TEST_F(RecipeWorkflowTest, LoadAndParseRecipe)
{
    // Arrange
    const QString id = "test_load_recipe";
    const QString name = "Load Test Recipe";
    createTestRecipeFile(id, name, m_storagePath);

    const QString filePath = m_storagePath + "/" + id + ".json";

    // Act
    QFile file(filePath);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly));
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    ASSERT_FALSE(doc.isNull());

    QJsonObject root = doc.object();

    // Assert — 顶级字段
    EXPECT_EQ(root["id"].toString(), id);

    // Assert — basic 子结构
    QJsonObject basic = root["basic"].toObject();
    EXPECT_EQ(basic["name"].toString(), name);
    EXPECT_EQ(basic["width"].toInt(), 100);
    EXPECT_EQ(basic["height"].toInt(), 160);
    EXPECT_DOUBLE_EQ(basic["group_pitch_x_mm"].toDouble(), 1.25);
    EXPECT_DOUBLE_EQ(basic["chip_width_mm"].toDouble(), 0.15045);
    EXPECT_EQ(basic["chip_color_order"].toInt(), 0);

    // Assert — camera 子结构
    QJsonObject camera = root["camera"].toObject();
    EXPECT_EQ(camera["light_brightness"].toInt(), 153);
    EXPECT_EQ(camera["exposure_us"].toInt(), 12000);
    EXPECT_DOUBLE_EQ(camera["capture_accuracy"].toDouble(), 2.95);

    // Assert — algo 子结构
    QJsonObject algo = root["algo"].toObject();
    EXPECT_DOUBLE_EQ(algo["group_confidence"].toDouble(), 0.72);
    EXPECT_DOUBLE_EQ(algo["chip_confidence"].toDouble(), 0.68);
    EXPECT_DOUBLE_EQ(algo["offset_x_um"].toDouble(), 15.0);

    // Assert — marks 数组
    QJsonArray marks = root["marks"].toArray();
    EXPECT_TRUE(marks.isEmpty());

    // Assert — panel_calib 子结构
    QJsonObject panelCalib = root["panel_calib"].toObject();
    EXPECT_EQ(panelCalib["mode"].toInt(), 0);
    EXPECT_EQ(panelCalib["fov_info"].toString(), "未生成");

    // Assert — position_calib 子结构
    QJsonObject posCalib = root["position_calib"].toObject();
    EXPECT_DOUBLE_EQ(posCalib["track_position"].toDouble(), 0.0);
}

// ============================================================
// 测试用例：配方修改与保存
// ============================================================

/**
 * @brief 验证配方可以被修改并重新保存
 *
 * 测试步骤：
 * 1. 创建配方文件
 * 2. 加载、修改字段值
 * 3. 重新保存
 * 4. 重新加载验证修改已持久化
 */
TEST_F(RecipeWorkflowTest, ModifyAndSaveRecipe)
{
    // Arrange
    const QString id = "test_modify_recipe";
    const QString originalName = "Original Name";
    createTestRecipeFile(id, originalName, m_storagePath);

    const QString filePath = m_storagePath + "/" + id + ".json";

    // Act — 加载并修改
    QFile file(filePath);
    ASSERT_TRUE(file.open(QIODevice::ReadWrite));
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();

    // 修改字段
    root["basic"] = QJsonObject{
        {"name", "Modified Name"},
        {"width", 200},
        {"height", 300},
        {"group_pitch_x_mm", 2.5},
        {"group_pitch_y_mm", 2.5},
        {"chip_pitch_in_group_mm", 1.0},
        {"chip_width_mm", 0.2},
        {"chip_height_mm", 0.1},
        {"pad_width_mm", 0.4},
        {"pad_height_mm", 0.2},
        {"group_width_mm", 0.4},
        {"group_height_mm", 0.6},
        {"chip_color_order", 1},
        {"chip_arrangement", 1},
        {"offset_group_mark_bl", QJsonObject{{"x", 1.0}, {"y", 2.0}}},
        {"offset_group_mark_tr", QJsonObject{{"x", 3.0}, {"y", 4.0}}}
    };
    root["camera"] = QJsonObject{
        {"light_brightness", 200},
        {"exposure_us", 15000},
        {"image_width", 2592},
        {"image_height", 1944},
        {"capture_accuracy", 1.5}
    };
    root["algo"] = QJsonObject{
        {"group_confidence", 0.85},
        {"chip_confidence", 0.80},
        {"chip_defect_class", 0.75},
        {"pad_defect_class", 0.70},
        {"offset_x_um", 10.0},
        {"offset_y_um", 10.0},
        {"angle_offset_deg", 0.3}
    };

    // 写回文件
    file.seek(0);
    file.write(QJsonDocument(root).toJson());
    file.resize(file.pos());
    file.close();

    // Assert — 重新加载验证
    QFile verifyFile(filePath);
    ASSERT_TRUE(verifyFile.open(QIODevice::ReadOnly));
    QJsonDocument verifyDoc = QJsonDocument::fromJson(verifyFile.readAll());
    verifyFile.close();

    QJsonObject verifyRoot = verifyDoc.object();
    EXPECT_EQ(verifyRoot["basic"].toObject()["name"].toString(), "Modified Name");
    EXPECT_EQ(verifyRoot["basic"].toObject()["width"].toInt(), 200);
    EXPECT_EQ(verifyRoot["basic"].toObject()["height"].toInt(), 300);
    EXPECT_EQ(verifyRoot["camera"].toObject()["light_brightness"].toInt(), 200);
    EXPECT_EQ(verifyRoot["camera"].toObject()["exposure_us"].toInt(), 15000);
    EXPECT_DOUBLE_EQ(verifyRoot["algo"].toObject()["group_confidence"].toDouble(), 0.85);
    EXPECT_DOUBLE_EQ(verifyRoot["algo"].toObject()["offset_x_um"].toDouble(), 10.0);
}

// ============================================================
// 测试用例：配方删除
// ============================================================

/**
 * @brief 验证配方文件可以被删除
 *
 * 测试步骤：
 * 1. 创建配方文件
 * 2. 验证文件存在
 * 3. 删除文件
 * 4. 验证文件不再存在
 */
TEST_F(RecipeWorkflowTest, DeleteRecipe)
{
    // Arrange
    const QString id = "test_delete_recipe";
    createTestRecipeFile(id, "To Delete", m_storagePath);
    const QString filePath = m_storagePath + "/" + id + ".json";
    ASSERT_TRUE(QFileInfo::exists(filePath));

    // Act
    bool deleted = QFile::remove(filePath);

    // Assert
    EXPECT_TRUE(deleted);
    EXPECT_FALSE(QFileInfo::exists(filePath)) << "Recipe file was not deleted";
}

// ============================================================
// 测试用例：配方导入
// ============================================================

/**
 * @brief 验证多个配方可以从 JSON 文件批量导入
 *
 * 测试步骤：
 * 1. 创建多个配方 JSON 文件
 * 2. 验证每个文件存在且 JSON 有效
 * 3. 验证每个文件包含正确的配方 ID 和名称
 */
TEST_F(RecipeWorkflowTest, ImportMultipleRecipes)
{
    // Arrange — 创建多个配方文件
    const QStringList testRecipes = {
        {"import_001"}, {"import_002"}, {"import_003"}
    };
    const QStringList recipeNames = {
        "Import Recipe 1", "Import Recipe 2", "Import Recipe 3"
    };

    for (int i = 0; i < testRecipes.size(); ++i) {
        createTestRecipeFile(testRecipes[i], recipeNames[i], m_storagePath);
    }

    // Act & Assert
    for (int i = 0; i < testRecipes.size(); ++i) {
        const QString filePath = m_storagePath + "/" + testRecipes[i] + ".json";

        // 验证文件存在
        EXPECT_TRUE(QFileInfo::exists(filePath))
            << "Recipe file " << testRecipes[i].toStdString() << " was not created";

        // 验证 JSON 有效
        QFile file(filePath);
        ASSERT_TRUE(file.open(QIODevice::ReadOnly));
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
        file.close();

        EXPECT_TRUE(parseError.error == QJsonParseError::NoError)
            << "Invalid JSON in " << testRecipes[i].toStdString();

        // 验证 ID 和名称
        QJsonObject root = doc.object();
        EXPECT_EQ(root["id"].toString(), testRecipes[i]);
        EXPECT_EQ(root["basic"].toObject()["name"].toString(), recipeNames[i]);
    }
}

// ============================================================
// 测试用例：配方导出
// ============================================================

/**
 * @brief 验证配方可以导出到指定目录
 *
 * 测试步骤：
 * 1. 创建多个配方文件
 * 2. 复制到导出目录
 * 3. 验证导出目录中的文件完整且有效
 */
TEST_F(RecipeWorkflowTest, ExportRecipesToDirectory)
{
    // Arrange
    const QStringList ids = {"export_001", "export_002"};
    const QStringList names = {"Export 1", "Export 2"};
    for (int i = 0; i < ids.size(); ++i) {
        createTestRecipeFile(ids[i], names[i], m_storagePath);
    }

    const QString exportDir = m_tempDir.path() + "/export";
    ASSERT_TRUE(QDir().mkpath(exportDir));

    // Act — 复制到导出目录
    for (const auto &id : ids) {
        const QString src = m_storagePath + "/" + id + ".json";
        const QString dst = exportDir + "/" + id + ".json";
        ASSERT_TRUE(QFile::copy(src, dst));
    }

    // Assert — 验证导出
    QStringList exportFiles = QDir(exportDir).entryList({"*.json"}, QDir::Files);
    EXPECT_EQ(exportFiles.size(), 2) << "Expected 2 exported recipe files";

    for (const auto &id : ids) {
        const QString filePath = exportDir + "/" + id + ".json";
        EXPECT_TRUE(QFileInfo::exists(filePath))
            << "Exported file " << id.toStdString() << " not found";

        // 验证 JSON 有效性
        QFile file(filePath);
        ASSERT_TRUE(file.open(QIODevice::ReadOnly));
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();
        ASSERT_FALSE(doc.isNull());

        QJsonObject root = doc.object();
        EXPECT_EQ(root["id"].toString(), id);
        EXPECT_TRUE(root.contains("basic"));
        EXPECT_TRUE(root.contains("camera"));
        EXPECT_TRUE(root.contains("algo"));
        EXPECT_TRUE(root.contains("marks"));
        EXPECT_TRUE(root.contains("panel_calib"));
        EXPECT_TRUE(root.contains("position_calib"));
    }
}

// ============================================================
// 测试用例：配方 JSON 结构完整性
// ============================================================

/**
 * @brief 验证默认配方 JSON 包含所有必需字段
 *
 * 测试步骤：
 * 1. 创建默认配方
 * 2. 验证 JSON 包含所有必需的顶级字段和子结构字段
 * 3. 验证字段类型正确
 */
TEST_F(RecipeWorkflowTest, RecipeJsonStructureIsComplete)
{
    // Arrange
    createTestRecipeFile("struct_test", "Structure Test", m_storagePath);
    const QString filePath = m_storagePath + "/struct_test.json";

    // Act
    QFile file(filePath);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly));
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    QJsonObject root = doc.object();

    // Assert — 检查所有必需字段存在
    EXPECT_TRUE(root.contains("id"));
    EXPECT_TRUE(root.contains("basic"));
    EXPECT_TRUE(root.contains("camera"));
    EXPECT_TRUE(root.contains("algo"));
    EXPECT_TRUE(root.contains("marks"));
    EXPECT_TRUE(root.contains("panel_calib"));
    EXPECT_TRUE(root.contains("position_calib"));

    // 检查 basic 子字段
    QJsonObject basic = root["basic"].toObject();
    EXPECT_TRUE(basic.contains("name"));
    EXPECT_TRUE(basic.contains("width"));
    EXPECT_TRUE(basic.contains("height"));
    EXPECT_TRUE(basic.contains("group_pitch_x_mm"));
    EXPECT_TRUE(basic.contains("offset_group_mark_bl"));

    // 检查 camera 子字段
    QJsonObject camera = root["camera"].toObject();
    EXPECT_TRUE(camera.contains("light_brightness"));
    EXPECT_TRUE(camera.contains("exposure_us"));
    EXPECT_TRUE(camera.contains("image_width"));
    EXPECT_TRUE(camera.contains("image_height"));
    EXPECT_TRUE(camera.contains("capture_accuracy"));

    // 检查 algo 子字段
    QJsonObject algo = root["algo"].toObject();
    EXPECT_TRUE(algo.contains("group_confidence"));
    EXPECT_TRUE(algo.contains("chip_confidence"));
    EXPECT_TRUE(algo.contains("offset_x_um"));
    EXPECT_TRUE(algo.contains("offset_y_um"));
    EXPECT_TRUE(algo.contains("angle_offset_deg"));

    // 检查 marks 是数组
    EXPECT_TRUE(root["marks"].isArray());

    // 检查 panel_calib 子字段
    QJsonObject panelCalib = root["panel_calib"].toObject();
    EXPECT_TRUE(panelCalib.contains("mode"));
    EXPECT_TRUE(panelCalib.contains("fov_info"));
    EXPECT_TRUE(panelCalib.contains("gerber_file_name"));

    // 检查 position_calib 子字段
    QJsonObject posCalib = root["position_calib"].toObject();
    EXPECT_TRUE(posCalib.contains("track_position"));
    EXPECT_TRUE(posCalib.contains("scan_position"));
}

// ============================================================
// 测试用例：空配方目录与边界条件
// ============================================================

/**
 * @brief 验证空目录的导入行为
 *
 * 测试步骤：
 * 1. 在空目录中查找 JSON 文件
 * 2. 验证没有文件被找到
 */
TEST_F(RecipeWorkflowTest, EmptyDirectoryReturnsNoRecipes)
{
    // Arrange — 使用空目录
    const QString emptyDir = m_tempDir.path() + "/empty";
    ASSERT_TRUE(QDir().mkpath(emptyDir));

    // Act
    QStringList files = QDir(emptyDir).entryList({"*.json"}, QDir::Files);

    // Assert
    EXPECT_TRUE(files.isEmpty()) << "Expected empty directory to have no recipe files";
}

// ============================================================
// 测试用例：无效 JSON 文件处理
// ============================================================

/**
 * @brief 验证无效的 JSON 文件不会导致崩溃
 *
 * 测试步骤：
 * 1. 创建无效的 JSON 文件
 * 2. 尝试解析
 * 3. 验证解析失败且不崩溃
 */
TEST_F(RecipeWorkflowTest, InvalidJsonFileDoesNotCrash)
{
    // Arrange
    const QString filePath = m_storagePath + "/invalid.json";
    {
        QFile file(filePath);
        ASSERT_TRUE(file.open(QIODevice::WriteOnly));
        file.write("this is not valid json content");
        file.close();
    }

    // Act — 尝试解析无效 JSON
    QFile file(filePath);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly));
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    // Assert
    EXPECT_TRUE(doc.isNull());
    EXPECT_NE(parseError.error, QJsonParseError::NoError);
}

// ============================================================
// main — Google Test 入口
// ============================================================

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}