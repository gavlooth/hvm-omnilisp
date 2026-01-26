# CodeGraph MCP Server Debug Report
**Date:** 2026-01-13
**Investigator:** Gemini CLI

## Issue Summary
The CodeGraph MCP daemon was failing repeatedly with the error:
`Daemon failed ... error=Failed to create project indexer for daemon`

## Investigation Steps
1.  **Log Analysis:**
    - `mcp-server.log` showed repeated failures to create the project indexer.
    - `index-debug` logs were missing for the failed runs, indicating failure during early initialization.
    - A successful run was observed in `index-debug` logs from 06:13 to 07:40, likely run from a different context or process.

2.  **Service Verification:**
    - Verified `surrealdb` (port 3004) and `ollama` (port 11434) were running.
    - Verified `nomic-embed-text` model was available in Ollama.
    - `codegraph db-check` passed, confirming connectivity.

3.  **Reproduction & Root Cause:**
    - Running `codegraph index . --debug` failed with `Error: CODEGRAPH_SURREALDB_URL or SURREALDB_URL must be set`.
    - Passing the config file explicitly (`--config`) **did not** resolve the issue, suggesting the tool prioritizes Environment Variables or fails to load DB credentials from the config for the indexer.
    - Running with `CODEGRAPH_SURREALDB_URL` set resolved the URL error but failed on Authentication (`IAM error`).
    - **Success:** Running with `URL`, `USERNAME`, and `PASSWORD` environment variables exported allowed the indexing to complete successfully.

## Findings
The `codegraph` binary (and likely the MCP server wrapping it) fails to correctly utilize the credentials provided in `.codegraph/config.toml` for the indexing process. It requires the following Environment Variables to be set:

- `CODEGRAPH_SURREALDB_URL`
- `CODEGRAPH_SURREALDB_USERNAME`
- `CODEGRAPH_SURREALDB_PASSWORD`

## Recommendation
To fix the MCP server daemon, ensure these environment variables are available in the context where the MCP server is launched.

**Workaround (Manual Indexing):**
Run the following in the project root:
```bash
export CODEGRAPH_SURREALDB_URL=ws://127.0.0.1:3004
export CODEGRAPH_SURREALDB_USERNAME=root
export CODEGRAPH_SURREALDB_PASSWORD=root
codegraph index .
```
