"""Reproduce same-input time/heap/quality comparisons with the C++ runner."""
import csv
from pathlib import Path
import subprocess
import tempfile
import sys

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'results/comparisons'
OUTPUT.mkdir(parents=True,exist_ok=True)
BINARY=ROOT/'build/social_bench'

with tempfile.TemporaryDirectory(prefix='nikita-comparisons-') as temp:
    temp=Path(temp)
    normal=temp/'normal'
    stress=temp/'stress'
    subprocess.run([str(ROOT/'build/social_cli'),'generate','12',str(normal)],check=True)
    subprocess.run([str(ROOT/'build/social_cli'),'generate','10000',str(stress)],check=True)
    adversarial=temp/'adversarial'
    adversarial.mkdir()
    (adversarial/'users.csv').write_text('id,name,campaign_cost\n0,creator_A,10\n1,creator_B,20\n2,creator_C,30\n')
    (adversarial/'topics.csv').write_text('id,name\n0,community\n')
    (adversarial/'posts.csv').write_text('id,user_id,topic_id,text\n0,0,0,community post A\n1,1,0,community post B\n2,2,0,community post C\n')
    (adversarial/'relationships.csv').write_text('from,to,delay,capacity\n0,1,1,10\n1,2,1,10\n2,0,1,10\n')
    (adversarial/'interactions.csv').write_text('from,to,post_id,timestamp,strength\n1,0,0,0,50\n2,1,1,1,90\n0,2,2,2,110\n')
    for label,directory,budget,shock in [('normal',normal,35,False),('shock',normal,10,True),('stress',stress,35,False),('adversarial_campaign',adversarial,50,False)]:
        args=[str(BINARY),'--scenario',str(directory),str(budget),str(OUTPUT/f'{label}.csv')]
        if shock:args.append('shock')
        subprocess.run(args,check=True)

lines=['# Same-input scenario comparisons','',
       'Generated with `make comparisons`. Each row uses three calls on identical scenario inputs. '
       'C++ heap payload is measured separately for each call; the table reports the largest peak and mean time. '
       'Source loading, input construction and exact-reference calculation are outside the timed/heap-metered call.','',
       '| Scenario | Strategy | Mean microseconds | Peak heap bytes | Value | Exact reference | Relative gap | Interpretation |',
       '|---|---|---:|---:|---:|---:|---:|---|']
for name in ['normal','shock','stress','adversarial_campaign']:
    rows=list(csv.DictReader((OUTPUT/f'{name}.csv').open()))
    for row in rows:
        if row['quality_kind'].startswith('exact_'):
            assert float(row['relative_gap'])==0,row
        assert int(row['measured_peak_heap_bytes'])>=0
        lines.append(f"| {name} | {row['algorithm']} | {float(row['mean_us']):.3f} | {int(row['measured_peak_heap_bytes']):,} | {float(row['solution_value']):.3f} | {float(row['reference_value']):.3f} | {100*float(row['relative_gap']):.2f}% | {row['quality_kind']} |")
    if name=='adversarial_campaign':
        by_name={r['algorithm']:r for r in rows}
        assert float(by_name['knapsack_dp']['solution_value'])==220
        assert float(by_name['density_greedy_integer']['solution_value'])==160
        assert float(by_name['fractional_knapsack']['solution_value'])==240
lines+=['','## What the results establish','',
        'Normal and shock are separate scenarios: all strategies within each scenario receive exactly the same '
        'active graph, candidate pool and budget. Stress contains 10,000 users and keeps the same top-20 '
        'candidate policy. Flow is omitted in stress because the dense residual solver is limited to 2,000 vertices.','',
        'Shortest-path, forest and exact campaign methods agree with their reference objective. The feasible '
        'density heuristic can lose quality: in the controlled three-creator case it scores 160 versus exact 220 '
        '(27.27% loss). The fractional result 240 is a 9.09% upper-bound gap, not a feasible integer solution. '
        'The controlled values map to actual network scores: one incoming edge contributes 10 and interaction '
        'strength contributes the remainder (50/90/110); costs are 10/20/30, with budget 50.','',
        'The relative gap is abs(value-reference)/max(1,abs(reference)); quality_kind identifies exact feasible '
        'solutions, feasible heuristics and fractional bounds so their guarantees are not confused. Input costs '
        'and values are not changed between strategies.','',
        '## Memory and timing method','',
        'The benchmark executable alone replaces C++ new/new[] and delete to count requested payload bytes. '
        'Each call starts a new measurement epoch. It includes algorithm allocations and returned buffers, '
        'and excludes pre-existing inputs, stack, allocator metadata, allocations performed only through C '
        'malloc and any other process. This is measured heap payload, not total resident memory. '
        'The wrapper adds allocation bookkeeping, so benchmark times are instrumented and may differ from '
        'the CLI. Run order, caches and OS scheduling still affect short measurements.','',
        'The ordinary benchmark CSV also retains a labeled analytical workspace estimate and cumulative '
        'process RSS; these are distinct from the per-call measured heap column. No subtracted RSS estimate '
        'is presented as an algorithm allocation measurement.','',
        '## Correctness and production selection','',
        'Dijkstra/Bellman-Ford compare complete distance vectors before timing. Prim/Kruskal compare forest '
        'weights. Knapsack DP supplies the reference; independent unit tests compare DP, backtracking and '
        'every bound policy with exhaustive small-instance oracles. Exact timed wrappers check equality. '
        'The domain workbench verifies matrix products, compression roundtrip, bounded all-pairs routes, '
        'CRT residues and all four text matchers on loaded records.','',
        'For nonnegative delays, prefer Dijkstra for its scalability guarantee; Bellman-Ford remains valuable '
        'for signed models and cycle detection. Choose DP when integer budgets fit its table, and exact search '
        'only for bounded candidate sets. Use a feasible heuristic only when its observed and worst-case '
        'quality limitations are acceptable; fractional allocation is an upper-bound diagnostic for indivisible '
        'creator contracts. Preserve the declared model and candidate limits in any production claim.','']
(OUTPUT/'REPORT.md').write_text('\n'.join(lines))
print('Wrote',OUTPUT/'REPORT.md')
