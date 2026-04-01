# Getting the Most Out of Claude Code (Token Efficiency Guide)

This is a practical guide for working with Claude Code when tokens are limited —
weekly plan caps, budget constraints, or just a preference not to burn through
usage on things that don't matter. These patterns apply to any Claude Code
session, not just Blended.

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
> "There's a bug in the tier system, can you look into it?"

Good:
> "In `scripts/startup/bl_ui/properties_material.py`, the `MATERIAL_PT_preview`
> panel still shows in Simple tier even though `blended_min_tier = 1`. The
> `poll()` method isn't calling `tier_at_least()`. Fix it."

The second version skips file discovery, hypothesis generation, and
clarification. Claude goes straight to the fix. That's probably 80% fewer
tokens.

### 2. Read files yourself before asking Claude to

If you already know the file, paste the relevant excerpt or say "read
`path/to/file` lines 40–80." Don't say "find where the tier check happens."
Searching costs tokens. Knowing costs zero.

### 3. Avoid agents for directed searches

Agents are great for open-ended research across many files. They're wasteful
for:

- "Find the definition of `tier_at_least`" → use Grep directly
- "Check if this file uses `template_list`" → use Grep directly
- "What's in `blended_defaults.py`?" → use Read directly

Agents add overhead (spawning, summarizing, returning). For anything you can
describe with a file path or a search term, skip the agent.

### 4. Use agents only for genuinely parallel or long-horizon work

Good agent uses:
- Researching upstream project history across many pages
- Exploring a codebase you've never seen before
- Running a multi-step task while you do something else (`run_in_background`)

Bad agent uses:
- Finding a single function
- Answering a question about one file you could just read

### 5. One task, one session

Sessions accumulate context. The longer a session runs, the more Claude has to
re-read to maintain coherence. If you've finished a logical unit of work
(a feature, a bug fix, a research task), commit, push, and start a fresh
session for the next thing. Clean context = fewer tokens per useful output.

### 6. Commit frequently, before context gets heavy

Every commit is a checkpoint. If you have to continue in a new session (or the
current one gets compressed), a clean commit history means Claude can quickly
reconstruct intent from `git log` rather than re-reading dozens of files. Small
commits also mean smaller diffs = faster code review passes.

### 7. Front-load constraints

Tell Claude what NOT to do at the start of a task, not after it's done
something wrong. Common ones for this project:

- "Don't spawn agents, use direct tool calls."
- "Don't refactor anything outside the specific function I'm asking about."
- "Keep the change to under 20 lines."
- "Don't add comments or docstrings."

Correcting a 200-line response you didn't want costs more than preventing it.

### 8. Ask for a plan before a big implementation

For anything touching more than 3 files, ask Claude to describe the approach
in one paragraph before writing any code. If the approach is wrong, course-
correct there — not after 400 lines of code have been written.

### 9. Narrow the scope of exploratory tasks

Bad:
> "Scan all recent upstream Blender commits and tell me what's relevant."

Better:
> "Fetch the Blender 5.1 release notes page and summarize only the UI and
> Python API changes."

The second version produces the same useful output with a fraction of the
tool calls. Scoping the question scopes the work.

### 10. Use CLAUDE.md to avoid re-explaining context

Every token Claude spends re-learning your project conventions is wasted. A
well-maintained `CLAUDE.md` means Claude arrives at each task already knowing:
- The file naming conventions
- Which files conflict on upstream merges
- The tier system pattern
- What NOT to do (the Critical Pitfalls section)

When you notice Claude doing something wrong repeatedly, add it to `CLAUDE.md`
rather than correcting it every session.

---

## Token Cost Reference (Rough Order of Magnitude)

| Operation | Relative Cost |
|-----------|--------------|
| Direct file read (Read tool) | Low |
| Direct grep/glob search | Low |
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
3. **One thing.** Pick the single highest-value task remaining and do only that.
4. **Short outputs.** Ask for concise responses ("in one paragraph", "under 20
   lines of code").
5. **Skip the docs.** Don't ask for comments, docstrings, or summaries unless
   they're the actual deliverable.
6. **Commit before you start.** If the session ends mid-task, you want a clean
   base to return to.

---

## What's Actually Worth Spending Tokens On

In rough priority order for this project:

1. **Bug fixes with a clear reproduction** — high value, well-scoped
2. **Implementing a specific feature you've already designed** — efficient if
   you provide the spec
3. **Upstream research** (like this document came from) — one-time cost,
   durable output
4. **Refactoring** — usually low priority; defer unless it's blocking something
5. **Exploration / "what does this code do?"** — use sparingly; read it yourself
   when you can

---

## The Meta-Principle

Claude Code works best when you treat it like a skilled contractor, not a
search engine. A contractor does their best work when you hand them a blueprint,
not when you ask them to figure out what to build. The more you've thought
through the task before opening a session, the more of your token budget goes
toward actual work rather than planning overhead.

> "Appreciate what already is." — the same principle applies here. Don't ask
> Claude to re-derive what you already know.
