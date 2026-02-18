#!/usr/bin/env bash

# Usage:
# Open Git Bash, cd into the repo or some sub-dir, run:
# ./git-monthly-stats.sh 2024-01-01 [2025-12-31] [branch] [--no-merges]

# First, verify we're inside a git repo
git rev-parse --is-inside-work-tree >/dev/null 2>&1 || {
  echo "Error: Not inside a git repository. Run from any sub-dir or root dir of a git repo."
  exit 1
}

# Read parameters passed
FROM="$1"
TO="${2:-now}" # if missing, default to now
BRANCH="$3" # if missing, default to remote master
NO_MERGES_FLAG=""

if [[ "$4" == "--no-merges" ]]; then
  NO_MERGES_FLAG="--no-merges"
fi

# Missing all params: output how-to
if [[ -z "$FROM" ]]; then
  echo "Usage: ./git-monthly-stats.sh <from-date> [to-date] [branch] [--no-merges]"
  echo "Run from inside a git repo; repo root or sub-dir."
  echo "Example: ./git-monthly-stats.sh 2025-01-01"
  echo "     or: ./git-monthly-stats.sh ""2025-01-01"" ""1 year ago"" origin/master"
  echo "     or: ./git-monthly-stats.sh 2025-01-01 2025-12-31 master"
  exit 1
fi
if [[ -z "$BRANCH" ]]; then
  # Get default remote branch (e.g. origin/master, origin/main)
  BRANCH=$(git symbolic-ref --short refs/remotes/origin/HEAD 2>/dev/null)
  if [[ -z "$BRANCH" ]]; then
    echo "Error: Default remote branch couldn't be looked up. Pass a branch explicitly."
    exit 1
  fi
fi

# Convert any relative date to absolute
FROM_ABS=$(date -d "$FROM" +%Y-%m-%d 2>/dev/null)
if [[ -z "$FROM_ABS" ]]; then
  echo "Error: Invalid FROM date: $FROM"
  exit 1
fi
TO_ABS=$(date -d "$TO" +%Y-%m-%d 2>/dev/null)
if [[ -z "$TO_ABS" ]]; then
  echo "Error: Invalid TO date: $TO"
  exit 1
fi

# Check if branch exists (local or remote)
if ! git show-ref --verify --quiet "refs/heads/$BRANCH" && \
   ! git show-ref --verify --quiet "refs/remotes/$BRANCH"; then
    echo "Error: Branch '$BRANCH' does not exist in this repository."
    exit 1
fi

# Get total commits
COMMITS=$(git rev-list --count $NO_MERGES_FLAG --since="$FROM_ABS" --until="$TO_ABS" "$BRANCH")

# Lines added / deleted (ignore binary files)
read ADDED DELETED <<< $(git log $NO_MERGES_FLAG --find-renames --since="$FROM_ABS" --until="$TO_ABS" \
  --pretty=tformat: --numstat "$BRANCH" \
  | awk '{ if ($1 ~ /^[0-9]+$/) { a+=$1; d+=$2 } } END { print a+0, d+0 }')

TOTAL_CHANGED=$((ADDED + DELETED))

# Files added / deleted
FILES_ADDED=$(git log $NO_MERGES_FLAG --find-renames --since="$FROM_ABS" --until="$TO_ABS" \
  --pretty=tformat: --name-status "$BRANCH" \
  | awk '$1=="A"{added++} END{print added+0}')

FILES_DELETED=$(git log $NO_MERGES_FLAG --find-renames --since="$FROM_ABS" --until="$TO_ABS" \
  --pretty=tformat: --name-status "$BRANCH" \
  | awk '$1=="D"{deleted++} END{print deleted+0}')

# Calculate number of months (inclusive)
MONTHS=$(( 
  ($(date -d "$TO_ABS" +%Y) - $(date -d "$FROM_ABS" +%Y)) * 12 +
  $(date -d "$TO_ABS" +%m) - $(date -d "$FROM_ABS" +%m) + 1
))

if [[ "$MONTHS" -le 0 ]]; then
  echo "Error: Invalid date range."
  exit 1
fi

AVG_COMMITS=$(awk "BEGIN {printf \"%.2f\", $COMMITS/$MONTHS}")
AVG_CHANGED=$(awk "BEGIN {printf \"%.2f\", $TOTAL_CHANGED/$MONTHS}")

echo ""
echo "Git Statistics"
echo "-------------------------------------------------"
echo "Repository : $(basename "$(git rev-parse --show-toplevel)")"
echo "Branch     : $BRANCH"
echo "Date range : $FROM_ABS → $TO_ABS"
echo "Months     : $MONTHS"
echo "-------------------------------------------------"
echo "Commits                : $COMMITS"
echo "Lines added            : $ADDED"
echo "Lines deleted          : $DELETED"
echo "Total lines changed    : $TOTAL_CHANGED"
echo "Files added            : $FILES_ADDED"
echo "Files deleted          : $FILES_DELETED"
echo "-------------------------------------------------"
echo "Average commits/month  : $AVG_COMMITS"
echo "Average lines changed/month : $AVG_CHANGED"
echo "-------------------------------------------------"
echo "Commits sorted by most to least line changes:"
echo ""
echo "Date       Hash    Lines Files Commit description"
echo "---------- ------- ----- ----- ------------------"
#    "YYYY-MM-DD hash7ch lines commitdesc"
git log --since="$FROM_ABS" --until="$TO_ABS" $NO_MERGES_FLAG \
  --find-renames \
  --date=short \
  --pretty=format:"%ad %h %s" \
  --numstat "$BRANCH" \
| awk '
  # Commit header line (starts with YYYY-MM-DD)
  /^[0-9]{4}-[0-9]{2}-[0-9]{2}/ {
    if (commit_seen) {
      print date, hash, total_lines, total_files, title
    }
    date=$1
    hash=$2
    title=substr($0, index($0,$3))
    total_lines=0
    total_files=0
    commit_seen=1
    next
  }

  # Numstat line (starts with number; counts as one file change
  /^[0-9]/ {
    if ($1 ~ /^[0-9]+$/ && $2 ~ /^[0-9]+$/) {
      total_lines += $1 + $2
      total_files += 1
    }
  }

  END {
    if (commit_seen) {
      print date, hash, total_lines, total_files, title
    }
  }
' \
| sort -k3,3nr -k4,4nr \
| awk '{ printf "%s %s %05d %05d %s\n", $1, $2, $3, $4, substr($0, index($0,$5)) }'
# Sort by lines count, then files count; files includes file add/delete commits
echo "-------------------------------------------------"
echo ""
