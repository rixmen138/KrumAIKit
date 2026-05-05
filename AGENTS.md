# AGENTS.md

This file contains instructions and context for any AI agents (or future developers) working on the KrumAIKit codebase.

## Codebase Architecture

KrumAIKit is an Editor-only plugin divided into several modules to enforce a clean separation of concerns:

- **KrumAIKitCore**: Contains shared types, interfaces (`IKrumAgent`, `IKrumTool`), and configurations. It has minimal dependencies.
- **KrumAIKitEditor**: Handles all UI elements (Slate). Central piece is `SKrumChatWindow`. Registers Editor commands and menus.
- **KrumAIKitAgents**: Manages agent process lifecycles. Contains wrappers for HTTP APIs (like `FKrumOpenRouterAgent`) and Subprocess launchers for CLI tools.
- **KrumAIKitTools**: Implementations of 30+ UE Editor tools (e.g., `FCreateBlueprintTool`, `FGetSelectedActorsTool`). All must implement `IKrumTool`.
- **KrumAIKitReaders**: Contains serializers that convert complex UE assets into JSON strings for AI context ingestion.

## Coding Standards

1. **Unreal Engine Standards**: Always follow standard UE5 C++ conventions (Prefixes: `F` for structs/non-UObject classes, `U` for UObjects, `A` for Actors, `S` for Slate widgets, `I` for Interfaces).
2. **Asynchronous Operations**: Never block the Game Thread. Process reading, heavy indexing, and HTTP requests must use `TaskGraph`, `FRunnable`, or UE's async HTTP module.
3. **Pointers**: Use `TSharedPtr`, `TSharedRef`, and `TWeakPtr`. Avoid raw owning pointers to prevent memory leaks.
4. **Error Handling**: All tool and agent executions must return sensible, properly formatted JSON errors instead of crashing. Validate all pointers (`IsValid()`, `!= nullptr`) before use.

## MCP Protocol
KrumAIKit tools are mapped directly to MCP. When adding a new tool:
1. Implement `GetSchema()` returning a valid JSON Schema object.
2. Ensure `Execute()` handles parsed `FJsonObject` parameters cleanly and returns a JSON string result.
