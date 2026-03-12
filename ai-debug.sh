#!/bin/bash
#===============================================================================
# File:        ai-debug.sh
#
# Description: CLI helper for AI VSCode Debug Proxy
#
#              Provides command-line interface for interacting with the
#              AI Debug Proxy HTTP API.
#
# Usage:       source ai-debug.sh
#              ai_launch ./build/app
#              ai_bp main.c 42
#              ai_continue
#
# Environment: AI_DEBUG_URL (default: http://localhost:9999)
#
#===============================================================================

# Default configuration
AI_DEBUG_URL="${AI_DEBUG_URL:-http://localhost:9999}"
AI_DEBUG_TIMEOUT="${AI_DEBUG_TIMEOUT:-30}"

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

#-------------------------------------------------------------------------------
# Internal helper functions
#-------------------------------------------------------------------------------

# Print colored message
_ai_print() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}"
}

# Print info message
_ai_info() {
    _ai_print "$BLUE" "ℹ $1"
}

# Print success message
_ai_success() {
    _ai_print "$GREEN" "✓ $1"
}

# Print warning message
_ai_warn() {
    _ai_print "$YELLOW" "⚠ $1"
}

# Print error message
_ai_error() {
    _ai_print "$RED" "✗ $1" >&2
}

# Execute debug operation
# Usage: _ai_debug_op <operation> [params_json]
_ai_debug_op() {
    local operation=$1
    local params=${2:-"{}"}
    
    local response
    response=$(curl -s -w "\n%{http_code}" -X POST "$AI_DEBUG_URL/api/debug" \
        -H "Content-Type: application/json" \
        -d "{\"operation\":\"$operation\",\"params\":$params}" \
        --max-time "$AI_DEBUG_TIMEOUT" 2>/dev/null)
    
    local http_code
    http_code=$(echo "$response" | tail -n1)
    local body
    body=$(echo "$response" | head -n -1)
    
    if [[ "$http_code" -eq 000 ]]; then
        _ai_error "Cannot connect to debug proxy at $AI_DEBUG_URL"
        return 1
    fi
    
    echo "$body"
    
    # Check if operation was successful
    local success
    success=$(echo "$body" | jq -r 'if .data.success != null then .data.success elif .success != null then .success else "true" end' 2>/dev/null)
    if [[ "$success" == "false" ]]; then
        return 1
    fi
    
    return 0
}

# Check if proxy is running
_ai_check_proxy() {
    local response
    response=$(curl -s -o /dev/null -w "%{http_code}" "$AI_DEBUG_URL/api/ping" --max-time 5 2>/dev/null)
    
    if [[ "$response" -eq 200 ]]; then
        return 0
    else
        return 1
    fi
}

#-------------------------------------------------------------------------------
# Public API functions
#-------------------------------------------------------------------------------

# Check if proxy is running
# Usage: ai_status
ai_status() {
    if _ai_check_proxy; then
        _ai_success "Debug proxy is running at $AI_DEBUG_URL"
        curl -s "$AI_DEBUG_URL/api/ping" | jq '.data'
    else
        _ai_error "Debug proxy is NOT running at $AI_DEBUG_URL"
        echo ""
        echo "Make sure the AI Debug Proxy extension is installed and active in VS Code."
        echo "You can start it with: Command Palette → AI Debug Proxy: Start Server"
        return 1
    fi
}

# Launch debug session
# Usage: ai_launch <program> [stopOnEntry] [workspacePath]
ai_launch() {
    local program=$1
    local stop_on_entry=${2:-true}
    local workspace_path=$3

    if [[ -z "$program" ]]; then
        _ai_error "Usage: ai_launch <program> [stopOnEntry] [workspacePath]"
        return 1
    fi

    # Auto-detect workspace path if program contains variables or is relative
    if [[ -z "$workspace_path" ]]; then
        if [[ "$program" == *'${workspaceFolder}'* ]] || [[ "$program" == *'${workspaceRoot}'* ]]; then
            # Try to detect workspace from current directory or git root
            if git rev-parse --show-toplevel &>/dev/null; then
                workspace_path=$(git rev-parse --show-toplevel)
                _ai_info "Auto-detected workspace from git root: $workspace_path"
            else
                workspace_path=$(pwd)
                _ai_info "Using current directory as workspace: $workspace_path"
            fi
        elif [[ "$program" != /* ]]; then
            # Relative path - use current directory as workspace
            workspace_path=$(pwd)
        fi
    fi

    _ai_info "Launching debug session for: $program"

    local params="{\"program\":\"$program\",\"stopOnEntry\":$stop_on_entry"
    if [[ -n "$workspace_path" ]]; then
        params="$params,\"workspacePath\":\"$workspace_path\""
    fi
    params="$params}"

    _ai_debug_op "launch" "$params"

    if [[ $? -eq 0 ]]; then
        _ai_success "Debug session launched"
    else
        _ai_error "Failed to launch debug session"
        return 1
    fi
}

# Set breakpoint
# Usage: ai_bp <file> <line> [condition]
ai_bp() {
    local file=$1
    local line=$2
    local condition=$3
    
    if [[ -z "$file" || -z "$line" ]]; then
        _ai_error "Usage: ai_bp <file> <line> [condition]"
        return 1
    fi
    
    local abs_path=$(readlink -f "$file" 2>/dev/null || echo "$file")
    local params="{\"location\":{\"path\":\"$abs_path\",\"line\":$line}}"
    if [[ -n "$condition" ]]; then
        params="{\"location\":{\"path\":\"$abs_path\",\"line\":$line},\"condition\":\"$condition\"}"
    fi
    
    _ai_info "Setting breakpoint at $file:$line"
    _ai_debug_op "set_breakpoint" "$params"
    
    if [[ $? -eq 0 ]]; then
        _ai_success "Breakpoint set"
    else
        _ai_error "Failed to set breakpoint"
        return 1
    fi
}

# Set temporary breakpoint
# Usage: ai_tbp <file> <line>
ai_tbp() {
    local file=$1
    local line=$2
    
    if [[ -z "$file" || -z "$line" ]]; then
        _ai_error "Usage: ai_tbp <file> <line>"
        return 1
    fi
    
    local abs_path=$(readlink -f "$file" 2>/dev/null || echo "$file")
    _ai_info "Setting temporary breakpoint at $abs_path:$line"
    _ai_debug_op "set_temp_breakpoint" "{\"location\":{\"path\":\"$abs_path\",\"line\":$line}}"
    
    if [[ $? -eq 0 ]]; then
        _ai_success "Temporary breakpoint set (will auto-remove on hit)"
    else
        _ai_error "Failed to set temporary breakpoint"
        return 1
    fi
}

# Continue execution
# Usage: ai_continue
ai_continue() {
    _ai_info "Continuing execution..."
    _ai_debug_op "continue" "{}"
    
    if [[ $? -eq 0 ]]; then
        _ai_success "Continued"
    else
        _ai_error "Failed to continue"
        return 1
    fi
}

# Step over (next line)
# Usage: ai_next
ai_next() {
    _ai_info "Stepping to next line..."
    _ai_debug_op "next" "{}"
    
    if [[ $? -eq 0 ]]; then
        _ai_success "Stepped"
    else
        _ai_error "Failed to step"
        return 1
    fi
}

# Step into function
# Usage: ai_step_in
ai_step_in() {
    _ai_info "Stepping into function..."
    _ai_debug_op "step_in" "{}"
    
    if [[ $? -eq 0 ]]; then
        _ai_success "Stepped in"
    else
        _ai_error "Failed to step in"
        return 1
    fi
}

# Step out of function
# Usage: ai_step_out
ai_step_out() {
    _ai_info "Stepping out of function..."
    _ai_debug_op "step_out" "{}"
    
    if [[ $? -eq 0 ]]; then
        _ai_success "Stepped out"
    else
        _ai_error "Failed to step out"
        return 1
    fi
}

# Get stack trace
# Usage: ai_stack
ai_stack() {
    _ai_info "Getting stack trace..."
    local result
    result=$(_ai_debug_op "stack_trace" "{}")
    
    if [[ $? -eq 0 ]]; then
        echo "$result" | jq '.data.frames'
    else
        _ai_error "Failed to get stack trace"
        return 1
    fi
}

# Get variables in current frame
# Usage: ai_vars [frameId]
ai_vars() {
    local frame_id=$1
    local params="{}"

    if [[ -n "$frame_id" ]]; then
        params="{\"frameId\":$frame_id}"
    fi

    _ai_info "Getting variables..."
    local result
    result=$(_ai_debug_op "get_stack_frame_variables" "$params")

    if [[ $? -eq 0 ]]; then
        # Extract variables from all scopes and flatten into single array
        echo "$result" | jq '[.data.scopes[]? | .variables[]?] | if length == 0 then null else . end'
    else
        _ai_error "Failed to get variables"
        return 1
    fi
}

# Evaluate expression
# Usage: ai_eval <expression>
ai_eval() {
    local expression=$1
    
    if [[ -z "$expression" ]]; then
        _ai_error "Usage: ai_eval <expression>"
        return 1
    fi
    
    _ai_info "Evaluating: $expression"
    local result
    result=$(_ai_debug_op "evaluate" "{\"expression\":\"$expression\"}")
    
    if [[ $? -eq 0 ]]; then
        echo "$result" | jq -r '.data.result // .error'
    else
        _ai_error "Failed to evaluate"
        return 1
    fi
}

# List source code around current line
# Usage: ai_source [lines_around]
ai_source() {
    local lines=${1:-5}
    
    _ai_info "Listing source (±$lines lines)..."
    local result
    result=$(_ai_debug_op "list_source" "{\"linesAround\":$lines}")
    
    if [[ $? -eq 0 ]]; then
        echo "$result" | jq -r '.data.sourceCode // .error'
    else
        _ai_error "Failed to list source"
        return 1
    fi
}

# Get last stop information
# Usage: ai_last_stop
ai_last_stop() {
    _ai_info "Getting last stop info..."
    local result
    result=$(_ai_debug_op "get_last_stop_info" "{}")
    
    if [[ $? -eq 0 ]]; then
        echo "$result" | jq '.data'
    else
        _ai_error "No stop event recorded"
        return 1
    fi
}

# Get active breakpoints
# Usage: ai_bps
ai_bps() {
    _ai_info "Getting active breakpoints..."
    local result
    result=$(_ai_debug_op "get_active_breakpoints" "{}")
    
    if [[ $? -eq 0 ]]; then
        echo "$result" | jq '.data.breakpoints'
    else
        _ai_error "Failed to get breakpoints"
        return 1
    fi
}

# Remove all breakpoints in file
# Usage: ai_clear_bps <file>
ai_clear_bps() {
    local file=$1
    
    if [[ -z "$file" ]]; then
        _ai_error "Usage: ai_clear_bps <file>"
        return 1
    fi
    
    _ai_info "Removing all breakpoints in $file"
    local abs_path=$(readlink -f "$file" 2>/dev/null || echo "$file")
    _ai_debug_op "remove_all_breakpoints_in_file" "{\"filePath\":\"$abs_path\"}"
    
    if [[ $? -eq 0 ]]; then
        _ai_success "Breakpoints removed"
    else
        _ai_error "Failed to remove breakpoints"
        return 1
    fi
}

# Quit debug session
# Usage: ai_quit
ai_quit() {
    _ai_info "Ending debug session..."
    _ai_debug_op "quit" "{}"
    
    if [[ $? -eq 0 ]]; then
        _ai_success "Debug session ended"
    else
        _ai_error "Failed to quit session"
        return 1
    fi
}

# Restart debug session
# Usage: ai_restart
ai_restart() {
    _ai_info "Restarting debug session..."
    _ai_debug_op "restart" "{}"
    
    if [[ $? -eq 0 ]]; then
        _ai_success "Session restarted"
    else
        _ai_error "Failed to restart session"
        return 1
    fi
}

# Pretty print a variable
# Usage: ai_pretty <expression>
ai_pretty() {
    local expression=$1
    
    if [[ -z "$expression" ]]; then
        _ai_error "Usage: ai_pretty <expression>"
        return 1
    fi
    
    _ai_info "Pretty printing: $expression"
    local result
    result=$(_ai_debug_op "pretty_print" "{\"expression\":\"$expression\"}")
    
    if [[ $? -eq 0 ]]; then
        echo "$result" | jq -r '.data.result // .error'
    else
        _ai_error "Failed to pretty print"
        return 1
    fi
}

# Get type of expression
# Usage: ai_type <expression>
ai_type() {
    local expression=$1
    
    if [[ -z "$expression" ]]; then
        _ai_error "Usage: ai_type <expression>"
        return 1
    fi
    
    _ai_info "Getting type of: $expression"
    local result
    result=$(_ai_debug_op "whatis" "{\"expression\":\"$expression\"}")
    
    if [[ $? -eq 0 ]]; then
        echo "$result" | jq -r '.data.type // .error'
    else
        _ai_error "Failed to get type"
        return 1
    fi
}

# Get function arguments
# Usage: ai_args
ai_args() {
    _ai_info "Getting function arguments..."
    local result
    result=$(_ai_debug_op "get_args" "{}")
    
    if [[ $? -eq 0 ]]; then
        echo "$result" | jq '.data.variables'
    else
        _ai_error "Failed to get arguments"
        return 1
    fi
}

# Jump to line
# Usage: ai_jump <line>
ai_jump() {
    local line=$1
    
    if [[ -z "$line" ]]; then
        _ai_error "Usage: ai_jump <line>"
        return 1
    fi
    
    _ai_info "Jumping to line $line"
    _ai_debug_op "jump" "{\"line\":$line}"
    
    if [[ $? -eq 0 ]]; then
        _ai_success "Jumped to line $line"
    else
        _ai_error "Failed to jump"
        return 1
    fi
}

# Run until specific line (sets temp breakpoint)
# Usage: ai_until <line>
ai_until() {
    local line=$1
    
    if [[ -z "$line" ]]; then
        _ai_error "Usage: ai_until <line>"
        return 1
    fi
    
    _ai_info "Running until line $line"
    _ai_debug_op "until" "{\"line\":$line}"
    
    if [[ $? -eq 0 ]]; then
        _ai_success "Running until line $line"
    else
        _ai_error "Failed to set until breakpoint"
        return 1
    fi
}

# List threads
# Usage: ai_threads
ai_threads() {
    _ai_info "Listing threads..."
    local result
    result=$(_ai_debug_op "list_threads" "{}")
    
    if [[ $? -eq 0 ]]; then
        echo "$result" | jq '.data'
    else
        _ai_error "Failed to list threads"
        return 1
    fi
}

# Switch to thread
# Usage: ai_switch_thread <threadId>
ai_switch_thread() {
    local thread_id=$1
    
    if [[ -z "$thread_id" ]]; then
        _ai_error "Usage: ai_switch_thread <threadId>"
        return 1
    fi
    
    _ai_info "Switching to thread $thread_id"
    _ai_debug_op "switch_thread" "{\"threadId\":$thread_id}"
    
    if [[ $? -eq 0 ]]; then
        _ai_success "Switched to thread $thread_id"
    else
        _ai_error "Failed to switch thread"
        return 1
    fi
}

# Get CPU registers
# Usage: ai_registers [frameId]
ai_registers() {
    local frame_id=$1
    local params="{}"
    
    if [[ -n "$frame_id" ]]; then
        params="{\"frameId\":$frame_id}"
    fi
    
    _ai_info "Getting registers..."
    local result
    result=$(_ai_debug_op "get_registers" "$params")
    
    if [[ $? -eq 0 ]]; then
        echo "$result" | jq '.data'
    else
        _ai_error "Failed to get registers"
        return 1
    fi
}

# Read memory
# Usage: ai_read_memory <address> <count>
ai_read_memory() {
    local address=$1
    local count=${2:-16}
    
    if [[ -z "$address" ]]; then
        _ai_error "Usage: ai_read_memory <address> [count]"
        return 1
    fi
    
    _ai_info "Reading $count bytes at $address"
    local result
    result=$(_ai_debug_op "read_memory" "{\"memoryReference\":\"$address\",\"offset\":0,\"count\":$count}")
    
    if [[ $? -eq 0 ]]; then
        echo "$result" | jq '.data'
    else
        _ai_error "Failed to read memory"
        return 1
    fi
}

# Disassemble code
# Usage: ai_disasm <address> [count]
ai_disasm() {
    local address=$1
    local count=${2:-20}
    
    if [[ -z "$address" ]]; then
        _ai_error "Usage: ai_disasm <address> [count]"
        return 1
    fi
    
    _ai_info "Disassembling $count instructions at $address"
    local result
    result=$(_ai_debug_op "disassemble" "{\"memoryReference\":\"$address\",\"instructionCount\":$count}")
    
    if [[ $? -eq 0 ]]; then
        echo "$result" | jq '.data'
    else
        _ai_error "Failed to disassemble"
        return 1
    fi
}

# Get document symbols (LSP)
# Usage: ai_symbols <file>
ai_symbols() {
    local file=$1
    
    if [[ -z "$file" ]]; then
        _ai_error "Usage: ai_symbols <file>"
        return 1
    fi
    
    _ai_info "Getting symbols in $file"
    local abs_path=$(readlink -f "$file" 2>/dev/null || echo "$file")
    local result
    result=$(curl -s "$AI_DEBUG_URL/api/symbols?fsPath=$abs_path" 2>/dev/null)
    
    if [[ $? -eq 0 ]]; then
        echo "$result" | jq '.data'
    else
        _ai_error "Failed to get symbols"
        return 1
    fi
}

# Execute batched debug operations
# Usage: ai_batch <operations_json> [parallel]
ai_batch() {
    local ops=$1
    local parallel=${2:-false}
    
    if [[ -z "$ops" ]]; then
        _ai_error "Usage: ai_batch <operations_json> [parallel]"
        return 1
    fi
    
    _ai_info "Executing batch operations (parallel=$parallel)..."
    
    local response
    response=$(curl -s -w "\n%{http_code}" -X POST "$AI_DEBUG_URL/api/debug/batch" \
        -H "Content-Type: application/json" \
        -d "{\"operations\":$ops,\"parallel\":$parallel}" \
        --max-time "$AI_DEBUG_TIMEOUT" 2>/dev/null)
    
    local http_code
    http_code=$(echo "$response" | tail -n1)
    local body
    body=$(echo "$response" | head -n -1)
    
    if [[ "$http_code" -eq 200 ]]; then
        _ai_success "Batch operations completed"
        echo "$body" | jq '.data'
    else
        _ai_error "Batch operations failed (HTTP $http_code)"
        echo "$body" | jq '.' 2>/dev/null || echo "$body"
        return 1
    fi
}

# Print help
# Usage: ai_help
ai_help() {
    cat << EOF
AI VSCode Debug Proxy - CLI Helper Functions
=============================================

Usage:
  source ai-debug.sh
  ai_<command> [args]

Session Management:
  ai_status              Check if proxy is running
  ai_launch <prog>       Launch debug session
  ai_quit                End debug session
  ai_restart             Restart session

Execution Control:
  ai_continue            Continue execution
  ai_next                Step over (next line)
  ai_step_in             Step into function
  ai_step_out            Step out of function
  ai_jump <line>         Jump to line
  ai_until <line>        Run until line

Breakpoints:
  ai_bp <file> <line> [cond]     Set breakpoint
  ai_tbp <file> <line>           Set temp breakpoint
  ai_bps                         List active breakpoints
  ai_clear_bps <file>            Remove all breakpoints in file

Inspection:
  ai_stack               Get stack trace
  ai_vars [frameId]      Get variables
  ai_args                Get function arguments
  ai_source [lines]      List source code
  ai_eval <expr>         Evaluate expression
  ai_pretty <expr>       Pretty print variable
  ai_type <expr>         Get type of expression
  ai_last_stop           Get last stop info

Advanced:
  ai_threads             List threads
  ai_switch_thread <id>  Switch to thread
  ai_registers [frameId] Get CPU registers
  ai_read_memory <addr>  Read memory
  ai_disasm <addr>       Disassemble code
  ai_symbols <file>      Get document symbols (LSP)
  ai_batch <ops_json>    Execute batched operations

Environment Variables:
  AI_DEBUG_URL     Proxy URL (default: http://localhost:9999)
  AI_DEBUG_TIMEOUT Request timeout in seconds (default: 30)

Examples:
  source ai-debug.sh
  ai_status
  ai_launch ./build/app
  ai_bp main.c 42
  ai_continue
  ai_stack
  ai_eval "my_variable"
  ai_quit

EOF
}

# Alias help to -h
ai_h() { ai_help; }

#-------------------------------------------------------------------------------
# Auto-check on source (optional)
#-------------------------------------------------------------------------------

# Uncomment to auto-check proxy on source:
# _ai_check_proxy || _ai_warn "Debug proxy not running at $AI_DEBUG_URL"

#-------------------------------------------------------------------------------
# End of ai-debug.sh
#-------------------------------------------------------------------------------
