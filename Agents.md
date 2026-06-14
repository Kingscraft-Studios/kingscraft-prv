# AGENTS.md

# Kingscraft Engine AI Agent Instructions

These instructions have the highest priority and apply to every task unless the user explicitly overrides them.

## 1. Read-Only by Default

Assume the project is read-only.

Do not modify any file unless the user explicitly requests changes and approves the plan.

## 2. Approval Required

Before editing anything:

- Explain what you want to do.
- List every file that will be modified.
- Explain why each file needs modification.
- Wait for explicit user approval.

Do not assume approval.

## 3. Never Delete Files

Never:

- Delete files
- Delete directories
- Rename files
- Move files
- Remove code because it appears unused
- Perform cleanup operations

Only do so if the user explicitly requests that exact action.

## 4. Never Touch Git

Do not execute any Git command unless the user explicitly asks.

Forbidden examples include:

- git commit
- git push
- git pull
- git merge
- git rebase
- git reset
- git checkout
- git switch
- git clean
- git stash
- git cherry-pick
- git revert
- git tag

Even read-only Git commands should not be executed unless requested.

## 5. No Automatic Refactoring

Do not:

- Reorganize folders
- Rename classes
- Rename files
- Change project architecture
- Format the entire codebase
- Remove warnings by deleting code
- Update unrelated code

Only modify what is necessary for the requested task.

## 6. Preserve Existing Code

Prefer extending existing systems.

Never rewrite large portions of code unless explicitly instructed.

If a rewrite seems beneficial, propose it first.

## 7. Keep Changes Minimal

Only modify files directly related to the task.

Avoid touching unrelated files.

## 8. Ask When Unsure

If there is ambiguity:

Stop.

Ask the user.

Do not guess.

## 9. Explain Plans First

Before implementation, provide:

- Objective
- Files to change
- Expected behavior
- Risks
- Alternative approaches (if applicable)

Wait for approval.

## 10. No Hidden Side Effects

Do not:

- Install packages
- Update dependencies
- Change build scripts
- Modify CI/CD
- Change compiler flags
- Download external code

Unless explicitly requested.

## 11. User Authority

The user's explicit instructions override these rules.

If there is any conflict or uncertainty, ask for clarification instead of acting.

## 12. Philosophy

Prefer safety over speed.

Prefer asking over assuming.

Prefer preserving user code over replacing it.

Avoid irreversible operations whenever possible.