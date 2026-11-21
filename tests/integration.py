"""Black-box regression and malformed-input tests; Python is test-only."""
import csv
import json
import pathlib
import shutil
import subprocess
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
CLI = ROOT / 'build/social_cli'
checks = 0

def run(*args, ok=True, stdin=None):
    global checks
    result = subprocess.run([str(CLI), *map(str, args)], input=stdin, text=True,
                            capture_output=True, timeout=30)
    assert (result.returncode == 0) == ok, (args, result.stdout, result.stderr)
    checks += 1
    return result

with tempfile.TemporaryDirectory(prefix='social-test-') as tmp:
    tmp = pathlib.Path(tmp)
    dataset = tmp / 'data'
    run('generate', 12, dataset)
    run('analyze', dataset, 35, tmp / 'normal')
    run('analyze', dataset, 10, tmp / 'shock', 'shock')
    normal = json.loads((tmp / 'normal.json').read_text())
    shock = json.loads((tmp / 'shock.json').read_text())
    assert normal['max_flow'] == 36 and shock['max_flow'] == 8
    assert normal['campaign_value'] == 472 and shock['campaign_value'] == 265
    assert normal['communities'] == 1 and shock['communities'] == 2
    assert len(normal['creator_topic_assignment']) == 3
    with (tmp / 'normal.csv').open() as f:
        rows = list(csv.DictReader(f))
    assert len(rows) == 12
    assert all(int(r['score']) == 10*int(r['incoming']) + int(r['engagement']) for r in rows)
    assert [r['id'] for r in normal['rankings']] == [int(r['id']) for r in rows]
    assert normal['rankings'][0]['score'] >= normal['rankings'][-1]['score']
    # Quoting roundtrip: commas, quotes, backslashes and Unicode.
    users = list(csv.reader((dataset / 'users.csv').open()))
    users[1][1] = 'Nikita, "creator" \\ café'
    with (dataset / 'users.csv').open('w', newline='') as f:
        csv.writer(f).writerows(users)
    run('analyze', dataset, 35, tmp / 'quoted')
    quoted = json.loads((tmp / 'quoted.json').read_text())
    assert next(r for r in quoted['rankings'] if r['id'] == 0)['name'] == users[1][1]
    # Each mutation starts from the same valid dataset.
    bad = tmp / 'bad'
    mutations = [
        ('users.csv', lambda s: s + s.splitlines()[1] + '\n'),
        ('users.csv', lambda s: s.replace('id,name,campaign_cost', 'bad,name,campaign_cost')),
        ('users.csv', lambda s: s.replace('0,', 'not-an-int,', 1)),
        ('users.csv', lambda s: s + '99,"unterminated,1\n'),
        ('relationships.csv', lambda s: s + '999999,0,1,1\n'),
        ('relationships.csv', lambda s: s + '0,1,-1,1\n'),
        ('relationships.csv', lambda s: s + '0,1,1,-1\n'),
        ('posts.csv', lambda s: s + '9999,0,999999,text\n'),
        ('interactions.csv', lambda s: s + '0,1,999999,10000,1\n'),
        ('interactions.csv', lambda s: s + '0,1,0,0,1\n'),
    ]
    for name, mutate in mutations:
        if bad.exists():
            shutil.rmtree(bad)
        shutil.copytree(dataset, bad)
        file = bad / name
        file.write_text(mutate(file.read_text()))
        run('analyze', bad, 35, tmp / 'invalid', ok=False)
    run('analyze', dataset, -1, tmp / 'invalid', ok=False)
    run('generate', 100001, tmp / 'invalid', ok=False)
    run('generate', '12oops', tmp / 'invalid', ok=False)
    run('analyze', tmp / 'missing', 35, tmp / 'invalid', ok=False)
    run('analyze', dataset, 35, tmp / 'invalid', 'typo', ok=False)
    run('menu', stdin='1\n' + str(dataset) + '\n3\n35\n' + str(tmp / 'menu') + '\n0\n')
    run('menu', stdin='')
    run('workbench')
    domain=run('workbench',dataset,35).stdout
    assert 'DATASET WORKBENCH: 12 users' in domain and 'Loaded-data campaign: exact DP/backtracking=472' in domain
    shocked=run('workbench',dataset,10,'shock').stdout
    assert 'Scenario=capacity_shock; budget=10' in shocked and 'CRT counter reconstruction=45' in shocked
    for method in ('naive', 'rk', 'fa', 'kmp'):
        assert run('match', method, 'aaaaa', 'aaa').stdout.splitlines() == ['0', '1', '2']
    run('match', 'typo', 'abc', 'a', ok=False)
    route_outputs = [run('route', dataset, 0, 11, method).stdout for method in ('dijkstra', 'bellman', 'floyd')]
    assert all(output.startswith('delay=') for output in route_outputs)
    assert len({output.split(';')[0].strip() for output in route_outputs}) == 1
    run('route', dataset, 999, 11, 'dijkstra', ok=False)
    run('route', dataset, 0, 11, 'typo', ok=False)
    run('demo', tmp / 'demo')
    stress = json.loads((tmp / 'demo/stress.json').read_text())
    assert stress['users'] == 10000 and stress['max_flow'] is None
    assert len(stress['rankings']) == 10000 and stress['campaign_candidate_count'] == 20
    checks += 13
print(f'Passed {checks} integration checks')
