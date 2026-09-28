#!/usr/bin/env bash
# Every check is followed by proof that it can go red, because a check that has never
# failed is a decoration
set -euo pipefail

usage() {
  cat <<'EOF'
check.sh — the whole gate

  check.sh [lint|behaviour|all]

  -h, --help   print this and exit

Two halves, because they need different things. `lint` reads what the repository ships
— the scripts, the workflows, SKILL.md and the vendored copies — with the linters the
flake's dev shell pins: actionlint, shellcheck, shfmt. `behaviour` runs check-prose.sh
on the readme and the fixtures and needs only bash and POSIX tools, so it runs under the
bash 3.2 macOS ships, which is what check-prose.sh claims to run on. `all`, the default,
is both

  nix develop -c ./tests/check.sh
  /bin/bash ./tests/check.sh behaviour        # on a macOS runner, CHECK_BASH32=1

Nothing here reaches the network
Exit 0 when everything holds, 1 on a finding, 2 on a usage error
EOF
}

HERE=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
cd "$HERE"

fail() {
  printf 'check: %s\n' "$1" >&2
  exit 1
}

# One source of truth for what gets linted: this list, read by nothing else
scripts=(tests/check.sh check-prose.sh check-skill.sh check-pins.sh check-sh.sh vendor-sync.sh)

# Every script below runs under the bash running this gate, not under whatever bash its
# shebang finds: on a macOS runner the gate is started as /bin/bash to prove the 3.2 macOS
# ships, while `env bash` would find Homebrew's 5
prose() { "$BASH" "$HERE/check-prose.sh" "$@"; }
# One place decides the mode, so no call is left asking for a tree the runner proving the
# 3.2 claim does not have: check-sh.sh reads the script it is given through shfmt, and a
# macOS image carries neither shfmt nor jq
# Whether check-sh.sh can read a script as a tree here, which is the one thing --bash-only
# is about. A macOS image carries neither shfmt nor jq, and neither does a job that runs
# the behaviour half alone; the machine is not the question, the tools are
tools_for_a_tree() { command -v shfmt >/dev/null 2>&1 && command -v jq >/dev/null 2>&1; }

checker() {
  local tree_flag=()
  tools_for_a_tree || tree_flag=(--bash-only)
  "$BASH" "$HERE/check-sh.sh" ${tree_flag[@]+"${tree_flag[@]}"} "$@"
}

# With a template, so a crashed run's leftovers say whose they are
work=$(mktemp -d "${TMPDIR:-/tmp}/check.XXXXXX")
trap 'rm -rf "$work"' EXIT

mode="${1:-all}"
case "$mode" in
  lint | behaviour | all) ;;
  -h | --help | help)
    usage
    exit 0
    ;;
  *)
    printf 'check: no such mode: %s — lint, behaviour or all\n' "$mode" >&2
    exit 2
    ;;
esac

tools=()
[[ "$mode" == behaviour ]] || tools+=(actionlint shellcheck shfmt)
missing=()
for tool in "${tools[@]+"${tools[@]}"}"; do
  command -v "$tool" >/dev/null || missing+=("$tool")
done
((${#missing[@]} == 0)) ||
  fail "missing: ${missing[*]} — they are pinned in the flake, so run this as: nix develop -c ./tests/check.sh"

check_lint() {
  echo "== the scripts parse and lint"
  for s in "${scripts[@]}"; do bash -n "$s"; done
  shellcheck "${scripts[@]}"
  shfmt -d -i 2 -ci "${scripts[@]}"

  echo "== the workflows pass actionlint"
  actionlint .github/workflows/*.yml

  echo "== the workflow lint is able to fail"
  mkdir -p "$work/bad/.github/workflows"
  cp tests/fixtures/must-fail.yml "$work/bad/.github/workflows/"
  if (cd "$work/bad" && actionlint .github/workflows/*.yml >/dev/null 2>&1); then
    fail "actionlint passed tests/fixtures/must-fail.yml — it cannot catch anything"
  fi

  echo "== the Nix this repository holds is formatted"
  # A `formatter` output nothing runs is a declaration, not a rule. nixfmt rather than
  # `nix fmt`, because the second needs the flake and this is the binary the wrapper calls.
  # find rather than a glob or git ls-files: a .nix in a subdirectory belongs here as much
  # as flake.nix, and a list taken from git is empty on a copy that carries no .git
  local nixfiles=()
  while IFS= read -r f; do nixfiles+=("$f"); done < <(find . -name '*.nix' -type f -not -path '*/.git/*')
  ((${#nixfiles[@]})) || fail "no .nix file is here, yet the flake declares a formatter"
  nixfmt --check "${nixfiles[@]}" ||
    fail "a .nix file here is not what nixfmt writes — run nix fmt"

  echo "== the vendored checkers are byte-equal to their source"
  # check-skill.sh comes from the skill-authoring skill
  # (https://github.com/rokokol/skill-authoring-skill), check-pins.sh from the ci skill
  # (https://github.com/rokokol/ci-skill) and check-sh.sh from the bash-best-practices
  # skill (https://github.com/rokokol/bash-best-practices-skill): every copy must still be
  # the blob .github/vendor.lock records, so one edited here instead of at its source
  # fails by name
  ./vendor-sync.sh check

  echo "== the workflows take no tool from a registry"
  # The ci skill's pin guard, which proves on every run that it catches each unpinned shape
  ./check-pins.sh

  echo "== SKILL.md loads, every reference is reachable, and every link and anchor resolves"
  # The one gate every skill repository shares: frontmatter, the name, links and anchors in
  # every doc, each check proven able to fail on a planted copy on every run
  ./check-skill.sh -n create-readme .
}

check_behaviour() {
  echo "== check-prose.sh answers to its own help"
  # The bash-best-practices skill's checker, vendored like the ones above: every flag and
  # exit code the script has is named in its help, held both ways. It
  # proves each of its own checks able to fail on every run. SKILL.md and the readme send
  # people to it, so every flag they give it has to be one it parses
  checker -m SKILL.md -m README.md check-prose.sh

  echo "== check-prose.sh refuses what it was not given"
  # A lint that exits 0 with nothing to read reads as a clean readme, and one that takes
  # --help for a path answers every question with "no such file"; both are a usage error
  refuses "no readme at all"
  refuses "an unknown flag" --nope README.md
  refuses "a readme that does not exist" README.md tests/fixtures/no-such-readme.md
  # Taken for a path, an unknown flag exits 2 as well, as a missing file, so the code alone
  # cannot tell the two apart: the usage is what says it was read as a flag
  out=$(prose --nope README.md 2>&1 || true)
  [[ "$out" == *"check-prose.sh DOC..."* ]] ||
    fail "check-prose.sh took an unknown flag for a document instead of printing its usage: $out"

  echo "== every document this repository ships obeys the rules it hands out"
  # Not the readme alone: the rules are the readme's and the reader is the same person, so
  # SKILL.md and the changelog are held to them too
  prose README.md SKILL.md CHANGELOG.md

  echo "== the proof of those rules travels with the file, and it notices a rule going dead"
  # The run above already proved them: check-prose.sh reads two documents of its own before
  # any real one — written to break every rule and to break none — and demands each finding
  # by name, six typographic marks by count and four full stops by count. The proof lives
  # inside the file because the file travels to repositories that keep no fixture for it.
  # What is left here is the proof of that proof: a copy with one rule taken away must fail
  # its own self-test, and for that rule's reason, or the self-test is passing on something
  # other than what it names
  neutered() { # neutered FRAGMENT REPLACEMENT WHAT EXPECTED
    local out
    # index and substr, not sub(): sub takes a regular expression, and a fragment holding
    # * or [ would either match nothing or match the wrong thing
    FRAG="$1" REPL="$2" awk '
      !done { at = index($0, ENVIRON["FRAG"]) }
      !done && at {
        $0 = substr($0, 1, at - 1) ENVIRON["REPL"] substr($0, at + length(ENVIRON["FRAG"]))
        done = 1
      }
      { print }
    ' check-prose.sh >"$work/neutered.sh"
    cmp -s check-prose.sh "$work/neutered.sh" &&
      fail "neutering $3 changed nothing in check-prose.sh — the edit matched no line"
    if out=$("$BASH" "$work/neutered.sh" README.md 2>&1); then
      fail "check-prose.sh with $3 taken away passed its own self-test — that proof proves nothing"
    fi
    [[ "$out" == *"$4"* ]] ||
      fail "check-prose.sh with $3 taken away failed for another reason: $out"
  }
  neutered "\$'\\xe2\\x80\\x9c'" "\$'\\xe2\\x80\\xNEVER'" \
    "the left double quotation mark" "of the 6 typographic marks"
  # shellcheck disable=SC2016 # the backtick is the character being taken out of the class
  neutered 'TAIL_MARKUP='"'"'*[*_)`"]'"'" "TAIL_MARKUP='*[*_)\"]'" \
    "a full stop behind a closing code span" "of the 4 full stops"
  neutered "'> [!'*']'?*" "'NEVERMATCHES'" \
    "the admonition shape" "admonition keyword"

  # check-prose.sh claims bash 3.2, and a grep for newer syntax is a proxy; the mechanism
  # is this half under the real 3.2, with two constructs planted that only a 3.2 rejects.
  # Under a newer bash they are no defect at all, so this block runs only where
  # CHECK_BASH32 says which bash this is, and first checks that claim
  if [[ -n "${CHECK_BASH32:-}" ]]; then
    echo "== this bash is the 3.2 the proof is about"
    ((BASH_VERSINFO[0] == 3)) ||
      fail "CHECK_BASH32 is set, but this is bash $BASH_VERSION — on macOS, run: /bin/bash ./tests/check.sh behaviour"
    ! "$BASH" -c 'declare -A m' >/dev/null 2>&1 || fail "CHECK_BASH32 is set, but this bash accepts declare -A"
    awk '{ print } /^set -euo pipefail$/ && !done { print "declare -A check_readme_probe || exit 70"; done = 1 }' \
      check-prose.sh >"$work/probe.sh"
    status=0
    "$BASH" "$work/probe.sh" README.md >/dev/null 2>&1 || status=$?
    ((status == 70)) || fail "a check-prose.sh that declares an associative array ran under this bash (got $status) — this is not a 3.2"
    # mapfile does not exist here, so a lint reading its readme with it dies on the spot
    # under set -e — the class of regression only this bash catches, since a newer one
    # runs the same line without a word
    sed 's/^  while IFS= read -r line; do$/  mapfile -t lines <\/dev\/null; while IFS= read -r line; do/' \
      check-prose.sh >"$work/mapfile.sh"
    grep -q 'mapfile -t lines' "$work/mapfile.sh" || fail "the mapfile plant did not land in check-prose.sh"
    # Nested, so the self-test does not answer first: it runs this same planted copy over
    # its own documents, dies on the same line, and reports a rule catching nothing —
    # true, and not what this plant is asking about
    out=$(CHECK_PROSE_NESTED=1 "$BASH" "$work/mapfile.sh" README.md 2>&1) &&
      fail "a check-prose.sh reading its readme with mapfile passed under this bash"
    [[ "$out" == *"mapfile: command not found"* ]] ||
      fail "the mapfile plant failed for a reason other than mapfile being absent: $out"
  fi
}

refuses() { # refuses WHAT ARGS... — check-prose.sh must exit 2 on ARGS
  local what=$1 rc=0
  shift
  prose "$@" >/dev/null 2>&1 || rc=$?
  ((rc == 2)) || fail "check-prose.sh exited $rc on $what, where a usage error is 2"
}

case "$mode" in
  lint) check_lint ;;
  behaviour) check_behaviour ;;
  all)
    check_lint
    check_behaviour
    ;;
esac

echo
echo "check: everything holds"
