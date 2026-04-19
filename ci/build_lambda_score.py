#!/usr/bin/env python3
import argparse
import json
from pathlib import Path


def load_json(path: str) -> dict:
    return json.loads(Path(path).read_text(encoding="utf-8"))


def merge_task_configs(task_configs: list[dict]) -> dict:
    merged = {
        "task": "Lambda",
        "pipeline_task_id": None,
        "max_score": 100,
        "suites": [],
    }

    suites_by_name = {}

    for config in task_configs:
        if merged["pipeline_task_id"] is None and config.get("pipeline_task_id") is not None:
            merged["pipeline_task_id"] = config.get("pipeline_task_id")
        if config.get("max_score") is not None:
            merged["max_score"] = config.get("max_score")

        for suite in config.get("suites", []):
            suites_by_name[suite["name"]] = {
                "name": suite["name"],
                "visibility": suite.get("visibility", "public"),
                "tasks": suite.get("tasks", []),
            }

    merged["suites"] = list(suites_by_name.values())
    return merged


def build_task_lookup(task_config: dict) -> dict:
    lookup = {}
    for suite in task_config.get("suites", []):
        suite_name = suite["name"]
        suite_lookup = {}
        for task in suite.get("tasks", []):
            suite_lookup[task["name"]] = int(task["points"])
        lookup[suite_name] = {
            "visibility": suite.get("visibility", "public"),
            "tasks": suite_lookup,
        }
    return lookup


def merge_reports(reports: list[dict]) -> dict:
    merged_suites = []
    summary_passed = 0
    summary_failed = 0
    summary_total = 0
    summary_success = True

    for report in reports:
        if "suites" not in report or not isinstance(report["suites"], list):
            raise SystemExit("Each report JSON must contain a 'suites' array")

        merged_suites.extend(report.get("suites", []))
        summary = report.get("summary", {})
        summary_passed += int(summary.get("passed", 0))
        summary_failed += int(summary.get("failed", 0))
        summary_total += int(summary.get("total", 0))
        summary_success = summary_success and bool(summary.get("success", False))

    return {
        "summary": {
            "passed": summary_passed,
            "failed": summary_failed,
            "total": summary_total,
            "success": summary_success,
        },
        "suites": merged_suites,
    }


def build_score(task_config: dict, report: dict) -> tuple[dict, str]:
    max_score = float(task_config.get("max_score", 100))
    task_lookup = build_task_lookup(task_config)

    suites_payload = []
    earned_points = 0
    total_points = 0
    total_passed = 0
    total_checks = 0

    for suite in report.get("suites", []):
        suite_name = suite.get("suite")
        suite_config = task_lookup.get(suite_name, {"visibility": "public", "tasks": {}})
        expected_tasks = suite_config["tasks"]
        suite_earned = 0
        suite_total = 0
        suite_passed = 0
        suite_checks = []

        for check in suite.get("checks", []):
            check_name = check.get("name")
            passed = bool(check.get("passed"))
            points = int(expected_tasks.get(check_name, 0))

            suite_total += points
            total_points += points
            total_checks += 1

            if passed:
                suite_earned += points
                earned_points += points
                suite_passed += 1
                total_passed += 1

            suite_checks.append(
                {
                    "name": check_name,
                    "passed": passed,
                    "points": points,
                    "details": check.get("details", ""),
                }
            )

        suites_payload.append(
            {
                "suite": suite_name,
                "visibility": suite_config["visibility"],
                "passed_checks": suite_passed,
                "total_checks": len(suite.get("checks", [])),
                "earned_points": suite_earned,
                "total_points": suite_total,
                "success": bool(suite.get("success")),
                "checks": suite_checks,
            }
        )

    normalized_score = 0.0
    if total_points > 0:
        normalized_score = round((earned_points / total_points) * max_score, 4)

    score_payload = {
        "task": "Lambda",
        "normalized_score": normalized_score,
        "earned_points": earned_points,
        "total_points": total_points,
        "passed_checks": total_passed,
        "total_checks": total_checks,
        "success": earned_points == total_points and total_points > 0,
        "suites": suites_payload,
    }

    markdown_lines = [
        "## Lambda Score",
        "",
        f"- Score: **{normalized_score:.2f} / {max_score:.2f}**",
        f"- Checks passed: **{total_passed} / {total_checks}**",
        f"- Points: **{earned_points} / {total_points}**",
        "",
    ]

    for suite in suites_payload:
        if suite.get("visibility") != "public":
            continue
        markdown_lines.append(
            f"### {suite['suite']}: {suite['earned_points']} / {suite['total_points']} points"
        )
        markdown_lines.append(
            f"Passed checks: {suite['passed_checks']} / {suite['total_checks']}"
        )
        markdown_lines.append("")

    return score_payload, "\n".join(markdown_lines) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--task-config", action="append", required=True)
    parser.add_argument("--report-json", action="append", required=True)
    parser.add_argument("--score-json", required=True)
    parser.add_argument("--markdown-output", required=True)
    args = parser.parse_args()

    task_configs = [load_json(path) for path in args.task_config]
    task_config = merge_task_configs(task_configs)
    reports = [load_json(path) for path in args.report_json]
    report = merge_reports(reports)
    score_payload, markdown = build_score(task_config, report)

    Path(args.score_json).write_text(
        json.dumps(score_payload, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    Path(args.markdown_output).write_text(markdown, encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
