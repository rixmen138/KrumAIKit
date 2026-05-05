# KrumAIKit (KAIK)

KrumAIKit is a full-featured AI agent integration plugin for Unreal Engine 5 (5.4 - 5.7). It serves as an open-source alternative and spiritual successor to NeoStack AI (Agent Integration Kit).

## Features
- **Integrated Chat Interface**: Slate-based, Markdown-supporting UI seamlessly embedded in the UE5 editor.
- **Multiple Agent Support**: Out-of-the-box support for Claude Code, Gemini CLI, OpenAI Codex, Cursor, Kilo Code, OpenRouter, and Ollama.
- **MCP Server Capabilities**: Exposes 30+ Unreal Editor tools to any MCP-compatible agent via stdio transport.
- **Deep Editor Integration**: Tools for Blueprints, Materials, Behavior Trees, State Trees, Sequencer, Niagara, and much more.

## Installation

1. Clone or download this repository into your project's `Plugins` folder: `[YourProject]/Plugins/KrumAIKit`.
2. Generate Visual Studio project files (Right-click your `.uproject` -> Generate Visual Studio project files).
3. Open your project in Visual Studio/Rider and compile.
4. Launch the Unreal Editor. Go to **Tools > KrumAI > Chat** (or press `Ctrl+Shift+K`) to open the interface.

## Quickstart

By default, KrumAIKit is ready to connect. If you wish to use an external tool like Claude Code:
1. Ensure the KrumAIKit plugin is enabled.
2. The plugin will automatically generate an `.mcp.json` file in your project root.
3. Launch your CLI tool (e.g., `claude`) from the project root. It will automatically detect KrumAIKit and have access to all Unreal Engine tools.
