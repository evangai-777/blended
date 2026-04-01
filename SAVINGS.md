# Getting the Most Out of Claude Code (Token Efficiency Guide)

A practical guide for working with Claude Code when tokens are limited —
weekly plan caps, budget constraints, or just a preference not to waste usage
on overhead. These patterns apply to any repo and any Claude Code session.

---

## The Core Idea

Tokens are spent on two things: **context** (what Claude reads) and **output**
(what Claude writes). Both matter. Most waste comes from one of three patterns:

1. Vague tasks that require many clarifying rounds
2. Agents launched for things a direct tool call could handle
3. Asking Claude to explore before you've told it where to look

---

## High-Leverage Patterns

### 1. Be specific about the target

Bad:
> "There's a bug in the authentication flow, can you look into it?"

Good:
> "In `src/auth/session.ts` around line 84, `validateToken()` isn't checking
> expiry before returning `true`. Add the expiry check."

The second version skips file discovery, hypothesis generation, and
clarification rounds. Claude goes straight to the fix — probably 80% fewer
tokens.

### 2. Know the file before asking Claude to find it

If you already know where something lives, say so. "Read `src/utils/parser.py`
lines 40–80" costs far less than "find where the parsing logic is." Searching
costs tokens. Knowing costs zero.

### 3. Avoid agents for directed searches

Agents are powerful but expensive. They're wasteful for anything with a clear
target:

- "Find the definition of `parseConfig`" → use Grep directly
- "Check if `utils.js` calls `fetch`" → use Grep directly
- "What's in `config/defaults.json`?" → use Read directly

Agents add overhead: spawning, sub-tool calls, summarizing, returning. For
anything you can describe with a file path or a symbol name, skip the agent.

### 4. Use agents only for genuinely open-ended work

Good agent uses:
- Researching an unfamiliar codebase you've never seen
- Scanning upstream project history across many pages/commits
- Running a multi-step background task while you do something else

Bad agent uses:
- Finding a single function or class definition
- Answering a question about one file you could just read
- Doing something a single Grep would handle

### 5. One task, one session

Sessions accumulate context. The longer a session runs, the more tokens are
spent on context overhead. When a logical unit of work is done (a bug fix, a
feature, a research task), commit, push, and start a fresh session for the
next thing. Clean context = fewer tokens per useful output.

### 6. Commit frequently, before context gets heavy

Every commit is a checkpoint. If you have to continue in a new session (or the
current one gets compressed), a clean commit history means Claude can quickly
reconstruct intent from `git log` rather than re-reading many files. Small,
descriptive commits also make code review faster.

### 7. Front-load constraints

Tell Claude what NOT to do at the start of a task — not after it has already
done it. Useful ones to internalize:

- "Don't spawn agents, use direct tool calls."
- "Don't refactor anything outside the specific function I'm asking about."
- "Keep the change under 20 lines."
- "Don't add comments, docstrings, or type annotations."
- "Don't add error handling for cases that can't happen."

Correcting an unwanted 200-line response costs more tokens than preventing it.

### 8. Ask for a plan before a big implementation

For anything touching more than 3 files, ask Claude to describe the approach
in one paragraph before writing any code. If the approach is wrong,
course-correct there — not after the implementation is already written.

### 9. Narrow the scope of exploratory tasks

Bad:
> "Scan all recent upstream changes and tell me what's relevant."

Better:
> "Fetch the v2.4 release notes page and summarize only the breaking API
> changes."

Scoping the question scopes the work. The second version produces the same
useful output with a fraction of the tool calls.

### 10. Use CLAUDE.md to avoid re-explaining context

Every token spent re-teaching Claude your project's conventions is wasted. A
well-maintained `CLAUDE.md` means Claude arrives at each task already knowing:

- File naming conventions and module structure
- Which files are sensitive or conflict-prone
- Patterns to follow and anti-patterns to avoid
- What NOT to do (the pitfalls section)

When you notice Claude doing something wrong repeatedly, add it to `CLAUDE.md`
rather than correcting it every session. One-time documentation cost, permanent
savings.

---

## Token Cost Reference (Rough Order of Magnitude)

| Operation | Relative Cost |
|-----------|--------------|
| Direct file read (Read tool) | Low |
| Direct Grep/Glob search | Low |
| Single WebFetch | Medium |
| Single WebSearch | Medium |
| Agent (foreground, simple task) | High |
| Agent (foreground, research task) | Very High |
| Agent (background, long-running) | Very High + waits |
| Large refactor across 10+ files | Very High |
| Back-and-forth correction loops | Compounds fast |

---

## Emergency Mode (Near the Cap)

When you're at ~10% remaining:

1. **No agents.** Every task uses direct tool calls only.
2. **No exploration.** Know the file before asking about it.
3. **One thing.** Pick the single highest-value task and do only that.
4. **Short outputs.** Ask for concise responses: "in one paragraph",
   "under 20 lines of code", "just the diff, no explanation."
5. **Skip the docs.** No comments, docstrings, or summaries unless they are
   the actual deliverable.
6. **Commit before you start.** If the session ends mid-task, you want a
   clean base to return to next week.

---

## What's Actually Worth Spending Tokens On

In rough priority order:

1. **Bug fixes with a clear reproduction** — high value, tight scope
2. **Implementing a feature you've already designed** — efficient when you
   provide the spec upfront
3. **One-time research with durable output** — pays for itself if you write
   down the findings (like this document)
4. **Refactoring** — low priority; defer unless it's actively blocking work
5. **Exploration / "what does this code do?"** — use sparingly; read it
   yourself when you can

---

## The Meta-Principle

Claude Code works best when you treat it like a skilled contractor, not a
search engine. A contractor does their best work when handed a blueprint, not
when asked to figure out what to build. The more you've thought through a task
before opening a session, the more of your token budget goes toward actual
work rather than planning overhead.

Hand Claude a blueprint. Don't ask it to design the building.
