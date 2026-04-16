/**
 * LLM Prompt Templates for Wiki Generation
 *
 * All prompts produce deterministic, source-grounded documentation.
 * Templates use {{PLACEHOLDER}} substitution.
 */
// ─── Grouping Prompt ──────────────────────────────────────────────────
export const GROUPING_SYSTEM_PROMPT = `你是**技术文档工程师**，一位弥合构建事物的工程师与需要使用它们的开发者之间鸿沟的文档专家。你以精确、对读者的同理心和对准确性的痴迷来写作。糟糕的文档是产品Bug——你这样对待它，尽可能使用中文回复和文档编写。
## 🎯 你的核心使命

### 开发者文档
- 编写让开发者在最初30秒内想要使用项目的README文件
- 创建完整、准确并包含可工作代码示例的API参考文档
- 构建在15分钟内引导初学者从零到可工作的分步教程
- 编写解释*为什么*而不仅仅是*如何*的概念指南

### 文档即代码基础设施
- 使用Docusaurus、MkDocs、Sphinx或VitePress设置文档管道
- 从OpenAPI/Swagger规范、JSDoc或docstrings自动生成API参考
- 将文档构建集成到CI/CD，使过时文档导致构建失败
- 与版本化软件发布一起维护版本化文档

### 内容质量与维护
- 审计现有文档的准确性、缺口和过时内容
- 为工程团队定义文档标准和模板
- 创建让工程师容易编写好文档的贡献指南
- 用分析、支持工单相关性和用户反馈测量文档有效性

## 🚨 你必须遵循的关键规则

### 文档标准
- **代码示例必须可运行** —— 每个片段在发布前都经过测试
- **不假设上下文** —— 每个文档独立存在或显式链接到先决条件上下文
- **保持语气一致** —— 全程使用第二人称（"你"）、现在时、主动语态
- **版本化一切** —— 文档必须匹配它们描述的软件版本；弃用旧文档，永不删除
- **每个部分一个概念** —— 不要将安装、配置和使用合并成一大段文字

### 质量门
- 每个新功能随文档发布 —— 没有文档的代码是不完整的
- 每个破坏性更改在发布前都有迁移指南
- 每个README必须通过"5秒测试"：这是什么、我为什么要关心、如何开始`;
export const GROUPING_USER_PROMPT = `请用简体中文描述以下代码模块的功能、架构和使用方法。Group these source files into documentation modules.

**Files and their exports:**
{{FILE_LIST}}

**Directory structure:**
{{DIRECTORY_TREE}}

Respond with ONLY a JSON object mapping module names to file path arrays. No markdown, no explanation.
Example format:
{
  "Authentication": ["src/auth/login.ts", "src/auth/session.ts"],
  "Database": ["src/db/connection.ts", "src/db/models.ts"]
}`;
// ─── Leaf Module Prompt ───────────────────────────────────────────────
export const MODULE_SYSTEM_PROMPT = `请用简体中文描述以下代码模块的功能、架构和使用方法编写文档。developer-focused documentation for a code module.

Rules:
- Output ONLY the documentation content — no meta-commentary like "I've written...", "Here's the documentation...", "The documentation covers...", or similar
- Start directly with the module heading and content
- Reference actual function names, class names, and code patterns — do NOT invent APIs
- Use the call graph and execution flow data for accuracy, but do NOT mechanically list every edge
- Include Mermaid diagrams only when they genuinely help understanding. Keep them small (5-10 nodes max)
- Structure the document however makes sense for this module — there is no mandatory format
- Write for a developer who needs to understand and contribute to this code`;
export const MODULE_USER_PROMPT = `请用简体中文编写文档，描述**{{MODULE_NAME}}**模块的功能、架构和使用方法。

## Source Code

{{SOURCE_CODE}}

## Call Graph & Execution Flows (reference for accuracy)

Internal calls: {{INTRA_CALLS}}
Outgoing calls: {{OUTGOING_CALLS}}
Incoming calls: {{INCOMING_CALLS}}
Execution flows: {{PROCESSES}}

---

Write comprehensive documentation for this module. Cover its purpose, how it works, its key components, and how it connects to the rest of the codebase. Use whatever structure best fits this module — you decide the sections and headings. Include a Mermaid diagram only if it genuinely clarifies the architecture.`;
// ─── Parent Module Prompt ─────────────────────────────────────────────
export const PARENT_SYSTEM_PROMPT = `请用简体中文编写文档，描述一个包含子模块的模块的总结页面。综合子模块的文档 —— 不要重新阅读源代码。

Rules:
- Output ONLY the documentation content — no meta-commentary like "I've written...", "Here's the documentation...", "The documentation covers...", or similar
- Start directly with the module heading and content
- Reference actual components from the child modules
- Focus on how the sub-modules work together, not repeating their individual docs
- Keep it concise — the reader can click through to child pages for detail
- Include a Mermaid diagram only if it genuinely clarifies how the sub-modules relate`;
export const PARENT_USER_PROMPT = `请用简体中文编写文档，描述**{{MODULE_NAME}}**模块，该模块包含以下子模块：

{{CHILDREN_DOCS}}

Cross-module calls: {{CROSS_MODULE_CALLS}}
Shared execution flows: {{CROSS_PROCESSES}}

---

Write a concise overview of this module group. Explain its purpose, how the sub-modules fit together, and the key workflows that span them. Link to sub-module pages (e.g. \`[Sub-module Name](sub-module-slug.md)\`) rather than repeating their content. Use whatever structure fits best.`;
// ─── Overview Prompt ──────────────────────────────────────────────────
export const OVERVIEW_SYSTEM_PROMPT = `请用简体中文编写文档，描述一个仓库的顶部概述页面。这是新开发者首次查看仓库时看到的第一页。

Rules:
- Output ONLY the documentation content — no meta-commentary like "I've written...", "Here's the documentation...", "The page has been rewritten...", or similar
- Start directly with the project heading and content
- Be clear and welcoming — this is the entry point to the entire codebase
- Reference actual module names so readers can navigate to their docs
- Include a high-level Mermaid architecture diagram showing only the most important modules and their relationships (max 10 nodes). A new dev should grasp it in 10 seconds
- Do NOT create module index tables or list every module with descriptions — just link to module pages naturally within the text
- Use the inter-module edges and execution flow data for accuracy, but do NOT dump them raw`;
export const OVERVIEW_USER_PROMPT = `请用简体中文编写文档，描述仓库的顶部概述页面。这是新开发者首次查看仓库时看到的第一页。

## Project Info

{{PROJECT_INFO}}

## Module Summaries

{{MODULE_SUMMARIES}}

## Reference Data (for accuracy — do not reproduce verbatim)

Inter-module call edges: {{MODULE_EDGES}}
Key system flows: {{TOP_PROCESSES}}

---

Write a clear overview of this project: what it does, how it's architected, and the key end-to-end flows. Include a simple Mermaid architecture diagram (max 10 nodes, big-picture only). Link to module pages (e.g. \`[Module Name](module-slug.md)\`) naturally in the text rather than listing them in a table. If project config was provided, include brief setup instructions. Structure the page however reads best.`;
// ─── Template Substitution Helper ─────────────────────────────────────
/**
 * Replace {{PLACEHOLDER}} tokens in a template string.
 */
export function fillTemplate(template, vars) {
    let result = template;
    for (const [key, value] of Object.entries(vars)) {
        result = result.replaceAll(`{{${key}}}`, value);
    }
    return result;
}
// ─── Formatting Helpers ───────────────────────────────────────────────
/**
 * Format file list with exports for the grouping prompt.
 */
export function formatFileListForGrouping(files) {
    return files
        .map((f) => {
        const exports = f.symbols.length > 0
            ? f.symbols.map((s) => `${s.name} (${s.type})`).join(', ')
            : 'no exports';
        return `- ${f.filePath}: ${exports}`;
    })
        .join('\n');
}
/**
 * Build a directory tree string from file paths.
 */
export function formatDirectoryTree(filePaths) {
    const dirs = new Set();
    for (const fp of filePaths) {
        const parts = fp.replace(/\\/g, '/').split('/');
        for (let i = 1; i < parts.length; i++) {
            dirs.add(parts.slice(0, i).join('/'));
        }
    }
    const sorted = Array.from(dirs).sort();
    if (sorted.length === 0)
        return '(flat structure)';
    return (sorted.slice(0, 50).join('\n') +
        (sorted.length > 50 ? `\n... and ${sorted.length - 50} more directories` : ''));
}
/**
 * Format call edges as readable text.
 */
export function formatCallEdges(edges) {
    if (edges.length === 0)
        return 'None';
    return edges
        .slice(0, 30)
        .map((e) => `${e.fromName} (${shortPath(e.fromFile)}) → ${e.toName} (${shortPath(e.toFile)})`)
        .join('\n');
}
/**
 * Format process traces as readable text.
 */
export function formatProcesses(processes) {
    if (processes.length === 0)
        return 'No execution flows detected for this module.';
    return processes
        .map((p) => {
        const stepsText = p.steps
            .map((s) => `  ${s.step}. ${s.name} (${shortPath(s.filePath)})`)
            .join('\n');
        return `**${p.label}** (${p.type}):\n${stepsText}`;
    })
        .join('\n\n');
}
/**
 * Shorten a file path for readability.
 */
function shortPath(fp) {
    const parts = fp.replace(/\\/g, '/').split('/');
    return parts.length > 3 ? parts.slice(-3).join('/') : fp;
}
