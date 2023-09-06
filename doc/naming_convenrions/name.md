# 项目开发中命名规则参考
>包含AI辅助创作
## 变量命名
**1. 变量名（Variables）：**

- 使用有意义的变量名，能够描述变量的用途。避免使用单个字符或无意义的名字。
- 使用小写字母，使用下划线 `_` 分隔单词，例如 `my_variable_name`。
- 对于布尔类型的变量，通常以 `is`、`has`、`can` 等前缀来表示，例如 `is_valid`、`has_data`。
- 避免使用具有特殊含义的单词，如关键字和库函数名称。

**2. 类名（Class Names）：**

- 使用驼峰命名法（CamelCase）：每个单词的首字母大写，不使用下划线。
- 类名应该具有描述性，能够清晰地表示类的用途，例如 `MyClass`、`HttpRequestHandler`。

**3. 常量名（Constants）：**

- 使用大写字母和下划线分隔单词来命名常量，例如 `MAX_VALUE`。
- 对于类内的常量成员，通常使用 `k` 或 `s` 作为前缀，例如 `static const int kMaxSize = 100;`。

**4. 函数名（Function Names）：**

- 使用小写字母，使用下划线 `_` 分隔单词，例如 `calculate_average`。
- 函数名应该清晰描述函数的功能。

**5. 命名空间（Namespace Names）：**

- 使用小写字母，不使用下划线。例如 `namespace mynamespace`。

**6. 枚举类型（Enum Types）：**

- 使用驼峰命名法，首字母大写。例如 `enum Color { Red, Green, Blue };`。

**7. 宏定义（Macro Definitions）：**

- 使用全大写字母，使用下划线分隔单词，例如 `#define MAX_BUFFER_SIZE 1024`。

**8. 类成员变量（Class Member Variables）：**

- 类成员变量通常以下划线 `_` 作为前缀，例如 `private int _count;`。也有一些团队将成员变量以 `m_` 作为前缀，例如 `private int m_count;`。

**9. 命名约定（Naming Conventions）：**

- 遵循团队或项目的命名约定，以确保一致性。
- 避免使用缩写，除非缩写是广泛认可的，例如 `HTTP`。
- 选择一种命名风格（例如驼峰命名法或下划线命名法）并保持一致性。

## 文件夹命名

1. **项目根文件夹（Project Root）：**

   - 使用有意义的项目名称作为根文件夹的名称，避免使用不相关或泛泛的名称。
   - 使用小写字母，使用连字符 `-` 或下划线 `_` 分隔单词，例如 `my-awesome-project` 或 `my_awesome_project`。

2. **子文件夹（Subfolders）：**

   - 子文件夹名称应该简洁且能够清晰地反映其内容或用途。
   - 使用小写字母，使用连字符 `-` 或下划线 `_` 分隔单词，例如 `src`、`assets`、`docs`。

3. **特定用途的文件夹（Special Purpose Folders）：**

   - 对于包含特定类型文件的文件夹，可以使用常见的约定，例如：
     - `src`：源代码文件夹。
     - `assets`：包含图像、样式表和其他资源文件的文件夹。
     - `docs`：包含文档和说明的文件夹。
     - `tests`：包含测试代码的文件夹。
     - `config`：包含配置文件的文件夹。

4. **版本控制和构建工具文件夹（Version Control and Build Tool Folders）：**

   - 对于版本控制和构建工具生成的文件夹，可以使用约定，例如：
     - `.git`：Git版本控制文件夹。
     - `.svn`：Subversion版本控制文件夹。
     - `.idea`：用于JetBrains IDE（如IntelliJ IDEA）的文件夹。
     - `node_modules`：用于Node.js项目的依赖文件夹。

5. **临时文件夹（Temporary Folders）：**

   - 对于临时或自动生成的文件夹，可以使用约定，例如：
     - `temp`：临时文件夹，用于存放临时文件。
     - `dist`：构建生成的文件夹，包含可部署的项目文件。

6. **多项目文件夹（Multi-Project Folders）：**

   - 如果您的根文件夹包含多个相关项目，可以使用子文件夹来组织它们，例如：
     - `project-1`：第一个项目的文件夹。
     - `project-2`：第二个项目的文件夹。
## 分支命名

1. **主分支（Main Branches）**：
   - `master`：用于存储稳定的、可部署的代码。这是主要的生产分支。
   - `develop`：用于开发新功能和进行集成测试的分支。通常是下一个发布版本的基础。

2. **特性分支（Feature Branches）**：
   - `feature/feature-name`：用于开发新功能的分支。例如，如果您正在开发登录功能，可以使用`feature/login`。
   - `bugfix/issue-name`：用于修复问题或缺陷的分支。例如，如果要修复一个称为“#123”的问题，可以使用`bugfix/123`。

3. **发布分支（Release Branches）**：
   - `release/version-number`：用于准备发布的分支。例如，如果您计划发布1.0版本，可以使用`release/1.0`。

4. **热修复分支（Hotfix Branches）**：
   - `hotfix/issue-name`：用于紧急修复生产问题的分支。例如，如果您需要快速修复一个紧急问题，可以使用`hotfix/urgent-issue`。

5. **支持分支（Support Branches）**：
   - `support/version-number`：用于支持特定版本的分支。例如，如果需要为1.0版本提供长期支持，可以使用`support/1.0`。

6. **个人分支（Personal Branches）**：
   - `yourname/feature-name`：有时，团队成员可能会创建个人分支来进行实验、测试或开发特定功能。使用个人名称和功能名称来命名，以防止与主要分支冲突。

7. **命名建议**：
   - 使用小写字母。
   - 使用连字符（减号）而不是下划线来分隔单词，例如`feature/login`而不是`feature_login`。
   - 尽量简明扼要但具有描述性，以便他人能够轻松理解分支的用途。
   - 避免使用特殊字符或空格。