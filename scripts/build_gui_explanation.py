"""Build the GUI explanation/presenter guide. Optional: python-docx, reportlab."""
from pathlib import Path
from xml.sax.saxutils import escape
from docx import Document
from docx.shared import Inches, Pt, RGBColor
from reportlab.platypus import SimpleDocTemplate, Paragraph, Table, TableStyle, PageBreak, Preformatted, Spacer
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.lib import colors
from reportlab.lib.pagesizes import A4

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'docs/presentation'
OUT.mkdir(parents=True,exist_ok=True)
pages=[
('Project introduction and starting the GUI',[
('p','Influence Studio is the graphical interface for the Social Network Influence and Community Analysis Tool, DAA Project 15. It helps explain influential users, communities, information-sharing paths and budget-constrained creator campaigns.'),
('say','Our project models a social network using weighted and capacitated graphs. The interface explains rankings and compares algorithms, while the actual assessed implementations run in C++17.'),
('h','How to start'),('code','make gui'),
('p','Run this command from the Nikita project folder. The browser opens http://127.0.0.1:8765. Keep the terminal running; Ctrl+C stops the server. On macOS, run_gui.command is a double-click alternative.'),
('code','python3 gui/server.py --port 8766 --no-browser'),
('p','Use this alternative if port 8765 is occupied; open http://127.0.0.1:8766 yourself. If Terminal reports getcwd: Operation not permitted, allow Terminal to access the Desktop folder in macOS privacy settings or use a checkout in an accessible directory.'),
('h','How it works'),('p','Browser HTML/CSS/JavaScript -> local Python standard-library HTTP bridge -> C++ CLI or benchmark runner -> reusable algorithm library. The Python layer handles requests and files; it does not replace the assessed C++ algorithms.'),
('h','What the initial screen means'),('p','The sample network loads automatically. Analytics engine ready indicates that the local engine is available. Local workspace means processing stays on this computer; it is not a logged-in social-platform account. The avatar is a visual workspace marker, not an authentication system.'),
('note','The following pages cover dashboard metrics, graph interaction, communities, campaigns, Algorithm lab, all 36 variants, comparisons, data/export, and a complete speaking script. Fill your own name, roll number and actual algorithm ownership before academic submission.')]),
('Top metrics, scenarios and campaign budget',[
('table',[['Display','Meaning','Normal sample'],['Network users','Total users in the loaded dataset','12'],['Communities','Fast-link weak components','1'],['Campaign value','Sum of selected creator influence scores','472'],['Dissemination capacity','Maximum flow from first to last user','36']]),
('h','Normal operation'),('p','Uses original relationship capacities and presets campaign budget to 35. This restores the standard sample demonstration conditions. Your own datasets may produce different results.'),
('h','Capacity shock'),('p','Divides every capacity by four, rounding down, and presets budget to 10. Edges reduced to zero stop carrying propagation traffic. Historical ranking factors remain unchanged; active routes and communities can change.'),
('table',[['Scenario','Budget','Campaign','Flow','Communities'],['Normal sample','35','472','36','1'],['Shocked sample','10','265','8','2'],['Stress: 10,000 users','35','1,233','Skipped','1,401']]),
('h','Campaign budget and Run analysis'),('p','Budget is the allowed total creator cost. Change the number and click Run analysis; editing the field alone does not change the current result. A zero budget selects no creators. Invalid inputs produce an error and leave the last successful analysis visible.'),
('say','These cards summarize one analyzed scenario. Normal and shock use different constraints, so their different objective values describe the shock rather than a difference in algorithm correctness.'),
('note','Updated just now reports elapsed analysis time for the engine request. It is different from the per-algorithm mean times in the comparison tool. Capacity units are synthetic model units, not measured messages per second.')]),
('Overview: graph, leaderboard and user spotlight',[
('h','Network explorer'),('table',[['Visual or control','Explanation'],['Circle','One displayed user'],['Number inside circle','Influence score, not user ID'],['Circle size','Larger scores produce larger nodes'],['Colour','Community membership'],['Directed connection','An active relationship; arrows show direction'],['Select a node','Updates the user spotlight and visible source-path highlight'],['Find a user','Filters/dims users in the displayed graph'],['Plus / minus / Fit','Zoom in, zoom out and restore the initial view']]),
('p','Parallel visual connections are collapsed to their lowest-delay active relationship. All original records still contribute to the engine calculations. Large graphs are sampled for display, with a disclosure; graph search covers displayed nodes. Use View all rankings to search the full network.'),
('h','Influence leaderboard'),('p','The top six ranked users are shown. View all rankings opens the complete searchable, paginated list. Rank position is not a user ID. Scores are ordered descending, with ascending user ID as the tie-break.'),
('code','Influence score = 10 x incoming relationships\n                  + incoming interaction strength'),
('h','User spotlight'),('p','Selecting a node or leaderboard row shows its name/ID, community, score, incoming links, incoming interaction strength and minimum-delay path from the source. The source is the first user in users.csv. A full textual path can include users omitted from a sampled graph.'),
('say','For user_2, six incoming relationships contribute 60 and incoming interaction strength contributes 32, so the score is 92. For user_11 in normal operation, the route is 0 -> 5 -> 10 -> 11 with total delay 6.'),
('note','This is a transparent influence heuristic, not proof of causal influence, unique audience reach or real-world popularity.')]),
('Communities: what the groups mean',[
('h','What to click'),('p','Open Communities in the sidebar. Browse group cards, search by member name or ID, and click a displayed member to return to their Overview spotlight. Groups are ordered by size and paginated.'),
('h','Community rule'),('code','Keep edges with capacity > 0 and delay <= 5.\nIgnore their direction for grouping only.\nEach connected component becomes a community.'),
('p','The propagation model remains directed. Direction is ignored only when computing these weak components. A single fast active bridge can join two otherwise separate groups.'),
('table',[['Card item','Meaning'],['Community number','Display label, not a quality ranking'],['Member count','Number of users in the component'],['Total score','Sum of the members influence scores'],['Member names','A bounded preview of members; large groups show a more-count'],['Search','Finds groups containing matching members']]),
('say','Our communities are defined by fast active connectivity. This is an explicit graph rule; it does not claim that members are friends or share the same interests.'),
('h','Why the shock changes the count'),('p','The sample has one community normally and two after the shock. A relationship whose capacity becomes zero can no longer connect those components. Ranking can stay unchanged because its historical records did not change.'),
('note','Colour and community labels aid inspection. This implementation does not optimize modularity or perform interest-based clustering.')]),
('Campaign planner and creator-topic matching',[
('h','What the planner answers'),('say','Given creator costs and a limited budget, which set produces the highest combined influence score?'),
('table',[['Column or result','Meaning'],['Creator','Candidate user name and external ID'],['Influence score','Value contributed if selected'],['Cost','Budget units required to select the creator'],['Decision','Selected or not selected by the exact solver'],['Budget used / remaining','Selected total cost and unused allowance'],['Campaign value','Sum of selected scores'],['Candidate pool','At most the top 20 ranked users']]),
('p','The solver is 0/1 knapsack dynamic programming: each creator is either fully selected or excluded. Fractional purchases are not allowed. The Overview campaign preview repeats the same plan and shows selected creator labels; it is not a separate optimization.'),
('h','Exactness and constraints'),('p','The selected total cost must not exceed budget. Exact optimality applies to the disclosed candidate pool, using an additive score. It does not mean an optimum over excluded users or a deduplicated audience-reach objective. Large DP tables are rejected instead of silently returning a partial answer.'),
('h','Creator-topic matching'),('p','This is a separate maximum bipartite matching across eligible creators and topics. A creator is eligible for a topic they actually posted about. At most one creator is assigned per topic and one topic per creator. It is not restricted to the selected campaign creators.'),
('say','Normal budget 35 gives campaign score 472. Shock budget 10 gives 265. We also compare this exact result with backtracking and three branch-and-bound policies on the same candidate items and budget.'),
('note','Audience overlap and causal reach are not modeled. Explain these limitations instead of describing campaign value as the number of people guaranteed to see a post.')]),
('Algorithm lab: routes and text matching',[
('h','Influence paths'),('p','Enter valid source and target user IDs, choose an algorithm and click Find influence path. IDs are external dataset identifiers, not ranking positions. Routing respects the currently analyzed normal/shock scenario.'),
('table',[['Strategy','What to explain'],['Dijkstra','Finds minimum-delay routes for nonnegative weights'],['Bellman-Ford','Supports signed analytical costs and detects reachable negative cycles'],['Floyd-Warshall','Computes all-pairs distances; GUI limits it to 600 users']]),
('p','On the sample, source 0 to target 11 has normal delay 6; Dijkstra/Bellman-Ford show 0 -> 5 -> 10 -> 11. Under shock, the minimum delay is 23 because the active graph changed. Compare strategies inside one scenario. Unreachable is a valid result when no active path exists.'),
('h','Text and event matching'),('p','Enter text and a pattern, choose a method and click Find matches. All methods return exact overlapping match positions. Positions count bytes; Unicode character positions may differ.'),
('table',[['Method','Core idea'],['Naive','Try every starting position'],['Rabin-Karp','Rolling hash with exact verification of hash hits'],['Finite automaton','Precompute state transitions for pattern prefixes'],['KMP','Reuse matched-prefix information to avoid repeated comparisons']]),
('code','Text: aaaaa       Pattern: aaa\nExpected offsets: 0, 1, 2'),
('say','All four strategies solve the same matching task. Correct answers should agree even when running time and preprocessing memory differ.'),
('note','An empty pattern matches every boundary, including the end. The display shows the first 500 positions plus the total count. GUI text/pattern limits are 60,000/4,000 bytes, respectively.')]),
('Loaded-data workbench: foundations and greedy methods',[
('p','Click Run workbench in Algorithm lab. It uses the selected dataset, scenario and budget. The output explains every analytical mapping and bound; it is not limited to a fixed demonstration network.'),
('table',[['Required variant','How to explain the result'],['Merge sort','Stable sorting of loaded network scores'],['Quick sort','Alternative sorting; results must agree with merge sort'],['Iterative binary search','Find a target score in the sorted array'],['Recursive binary search','Same search contract through recursive halving'],['Maximum subarray','Find an engagement window above the integer mean in up to 256 interactions'],['Fast powering','Recursive squaring for a bounded growth-factor exercise; not a reach forecast'],['Celebrity discovery','Recursive candidate elimination and verification among at most eight users'],['Classical matrix multiply','Compute a two-step active-link projection'],['Strassen matrix multiply','Compute the same product with seven recursive half-size products'],['Activity selection','Choose compatible mapped posting intervals'],['Interval partitioning','Find the required concurrent channels for those intervals'],['Fractional knapsack','Compute a divisible-allocation upper bound'],['Huffman coding','Encode loaded post text and verify a lossless roundtrip'],['Dijkstra','Compute routes on the full active network'],['Prim','Minimum forest in the undirected infrastructure projection'],['Kruskal','Alternative minimum forest; total weight must agree']]),
('note','Posting durations are the analytical rule 1 + text-byte-length mod 5; starts use the first interaction timestamp mod 10,000 or a record-index fallback. Matrices/celebrity use bounded user projections. These assumptions are printed, not presented as measured platform behavior.')]),
('Loaded-data workbench: DP, exact search, flow and text',[
('table',[['Required variant','What it demonstrates'],['0/1 knapsack DP','Exact campaign over the same costs/scores/budget'],['Matrix-chain DP','Order degree-derived analytics projections cheaply'],['LCS','Common subsequence of the first two users post text; reports byte length'],['Optimal BST','Weighted lookup cost from engagement hits and declared analytical miss weights'],['Bellman-Ford','Full-network route agreement with Dijkstra'],['Floyd-Warshall','All-pairs check on the bounded induced graph'],['Bounded TSP','Exact direct-link tour over at most eight users; no feasible tour is valid'],['N-Queens','Explicit syllabus backtracking validation, not a social prediction'],['Sum of subsets','Exact-budget combinations from at most 16 candidate costs'],['Knapsack backtracking','Exact inclusion/exclusion search on the actual candidate pool'],['FIFO branch and bound','Queue exploration with a safe fractional upper bound'],['LIFO branch and bound','Stack exploration of the same decision'],['Least-cost branch and bound','Explore the best profit bound first'],['Ford-Fulkerson','Integer dissemination flow and residual capacity'],['Maximum bipartite matching','Creator-topic assignment using actual posting eligibility'],['Chinese Remainder Theorem','Reconstruct actual interaction-counter residues modulo 105'],['Naive matching','Baseline matching on loaded text'],['Rabin-Karp matching','Hash-assisted exact matching on the same text'],['Finite-automaton matching','State-table matching on the same text'],['KMP matching','Prefix-based matching on the same text']]),
('say','Together these two pages list all 36 mandatory variants. Expensive problems use clearly declared projections; N-Queens is a validation exercise. The loaded-data workbench checks agreement where alternatives solve the same task.'),
('note','The sample has 45 interactions: residues modulo 3, 5 and 7 reconstruct 45 modulo 105. Larger counters are reconstructed modulo the modulus product, not uniquely over all integers. All four loaded-text matchers use the first topic keyword, bounded to 128 bytes.')]),
('Compare this network: time, memory and solution quality',[
('p','Click Compare this network to run strategies on the current dataset, active graph, candidate pool and budget. It downloads comparison CSV and prints results. This differs from Run benchmarks, which uses predefined reproducible experiment datasets.'),
('table',[['CSV field','What to say'],['mean_us / best_us','Mean and best time over three repetitions, in microseconds'],['measured_peak_heap_bytes','Largest per-call live C++ new/new[] payload over those repetitions'],['estimated_workspace_bytes','Separate analytical estimate; blank means not provided'],['process_peak_rss_bytes','Cumulative process high-water memory; not per-algorithm allocation'],['solution_value / reference_value','Observed objective and exact reference where applicable'],['relative_gap','abs(value-reference) / max(1,abs(reference))'],['quality_kind','Exact solution, feasible heuristic, fractional upper bound or unreachable']]),
('h','Controlled quality example'),('table',[['Strategy','Value','Interpretation'],['Exact DP / exact search','220','Feasible optimum'],['Integer density greedy','160','Feasible but 27.27% below optimum'],['Fractional knapsack','240','Upper bound 9.09% above optimum; not an integer plan']]),
('p','The controlled campaign has costs 10/20/30, values 60/100/120 and budget 50. It is generated by make comparisons; it is not the normal sample campaign whose optimum is 472.'),
('say','A zero objective gap shows agreement for the measured instance, not proof that every possible input is correct. Feasibility tests and independent small-instance oracles support the correctness claim.'),
('note','Heap measurement excludes stack, pre-existing inputs, allocator metadata and malloc-only allocations. Allocation bookkeeping affects timings. Three repetitions do not establish statistical confidence. Use make comparisons for normal/shock/stress and the controlled counterexample.')]),
('Datasets, exports, benchmarks and session limits',[
('h','Change dataset'),('p','Use the sample; generate 2 to 10,000 synthetic users; load a local directory; or select all five CSV files together. Relative directories are resolved from the repository root. The C++ loader validates the schema and references.'),
('table',[['CSV file','Expected columns'],['users.csv','id,name,campaign_cost'],['relationships.csv','from,to,delay,capacity'],['posts.csv','id,user_id,topic_id,text'],['topics.csv','id,name'],['interactions.csv','from,to,post_id,timestamp,strength']]),
('p','IDs must be valid and references must exist. Costs and delays are positive, capacities nonnegative, and interaction timestamps nondecreasing. Imports are limited to 12 MB and 10,000 users. All five files are required even when some contain only headers. Source files are not overwritten; the GUI uses a snapshot.'),
('h','Export report'),('p','Export JSON for rankings, score factors, communities, paths, campaign selections, assignments and flow. Export CSV for a ranked row per user, including factors, community and delay. Downloads are original C++ report formats; extra GUI layout/cost metadata is not added. Save exports before stopping the server.'),
('h','Run benchmarks'),('p','Runs 144 predefined cases across all 36 mandatory variants on small, medium, large, adversarial and bounded families, with three repetitions each. It downloads CSV. Use Compare this network when you specifically want the current loaded input.'),
('h','Important limits'),('p','The visual graph shows at most 102 selected users and up to 1,000 active edges before parallel-line collapsing. Full calculations and searchable rankings use all users. Dense flow is skipped above 2,000 users; Floyd-Warshall is capped at 600. The GUI retains up to 50 datasets and 100 reports per session, allows two engine jobs, and times out a job after 90 seconds.'),
('note','Generated/snapshotted datasets are temporary and removed when the GUI stops normally. Downloaded files persist. Exact algorithms have additional table/instance limits documented in docs/DESIGN.md.')]),
('A complete presentation script: click, explain, verify',[
('table',[['Step / time','Click or action','Suggested words'],['1 / 1 min','Start make gui','This is a local GUI backed by our reusable C++ algorithm library.'],['2 / 1 min','Show Overview cards','The sample has 12 users, one community, campaign value 472 and flow 36.'],['3 / 1 min','Select user_2, then user_11','Score 92 is 10 times six incoming links plus strength 32. User_11 has normal path delay 6.'],['4 / 1 min','Communities and Campaign planner','Groups follow the fast-link rule; the campaign is an exact budget-feasible selection over its candidate pool.'],['5 / 1 min','Capacity shock','Capacities are quartered and budget becomes 10. Flow falls to 8; campaign is 265; two communities remain.'],['6 / 1 min','Restore sample/normal; Algorithm lab','For IDs 0 and 11 all route strategies agree at delay 6. All four matchers find 0,1,2 for aaaaa/aaa.'],['7 / 1 min','Run workbench / Compare this network','We exercise the syllabus on loaded data and compare time, measured heap, objective gaps and guarantees.'],['8 / 1 min','Normal; generate 10,000 users','The stress case still returns full rankings and paths. Its score is 1233; flow is skipped by an explicit size cap.'],['9 / 1 min','Export report','Results can be downloaded as original C++ CSV/JSON for inspection.'],['10 / 1 min','Discuss tests and limits','We verify answers independently and state model, measurement and size limits.']]),
('h','Evidence to quote'),('p','15,265 C++ unit/oracle checks; 4 direct heap-meter invariants; 48 CLI integration checks; 8 grouped GUI HTTP tests; 64 automated browser workflow/layout checks; 60,678 additional end-to-end assertions; 144 benchmark cases. Sanitizer scope is recorded in results/GUI_VALIDATION.txt.'),
('say','The value of the project is that its decisions are explainable and its strategies can be compared on the same data. We demonstrate normal behavior, reduced capacity and large-data behavior, and report limitations with the results.'),
('note','Restore Normal operation before generating the stress dataset so budget is 35. Restore the 12-user sample before comparing Floyd-Warshall; a 10,000-user all-pairs request correctly exceeds its limit.')]),
('Viva answers, limitations and fallback commands',[
('table',[['Question','Answer to explain'],['Is the GUI itself written in C++?','The algorithm engine/library are C++17. The optional browser GUI uses local HTML/CSS/JS and a Python standard-library bridge.'],['Is the score real-world influence?','It is a transparent degree/engagement heuristic, not causal proof or guaranteed reach.'],['Why is fractional value higher?','Partial items relax the indivisible decision, producing an upper bound rather than a legal creator plan.'],['Why do communities change after shock?','Zero-capacity links disappear from active connectivity; historical ranking records stay unchanged.'],['Why can Bellman-Ford be faster on some data?','Input order, early stopping and setup constants matter; worst-case guarantees do not dictate every measured run.'],['What is the knapsack complexity classification?','General decision knapsack is weakly NP-complete; optimization is NP-hard. The capped top-20 program is a restricted instance family.'],['What is the reduction?','From SUBSET SUM: cost_i=value_i=a_i and budget=threshold=T. A feasible selection must sum exactly to T.'],['Why is DP pseudo-polynomial?','O(kB) depends on numeric budget B, whose binary representation needs O(log B) bits.'],['What does zero quality gap prove?','Agreement for that objective/instance; other tests verify feasibility and independent correctness.'],['What is still student-provided?','Actual individual ownership, human review and a recorded presentation; these are not fabricated.']]),
('h','Fallback commands'),('code','make gui\n./build/social_cli\n./build/social_cli workbench data/sample 35\n./build/social_cli demo\nmake comparisons\nmake test'),
('note','Keep sanitizer rebuilds outside the timed presentation. Campaign exactness is limited to an additive top-20 pool; communities use threshold weak components. Do not describe synthetic capacities as measured platform throughput or peak heap payload as total process memory. Full guides and evidence are in docs/GUI.md, docs/REQUIREMENTS.md, docs/COMPLEXITY.md and results/comparisons/REPORT.md.')])
]

navy='173346';teal='087F73';gray='536B79'
doc=Document();sec=doc.sections[0]
sec.top_margin=sec.bottom_margin=Inches(.6);sec.left_margin=sec.right_margin=Inches(.65)
style=doc.styles['Normal'];style.font.name='Calibri';style.font.size=Pt(10)
style.paragraph_format.space_after=Pt(6)
for name in ['Title','Heading 1','Heading 2']:doc.styles[name].font.color.rgb=RGBColor.from_string(teal if name=='Heading 2' else navy)
for i,(title,blocks) in enumerate(pages):
    if i:doc.add_page_break()
    doc.add_heading(f'{i+1:02d}  {title}',0)
    for kind,value in blocks:
        if kind=='h':doc.add_heading(value,2)
        elif kind=='table':
            table=doc.add_table(rows=0,cols=len(value[0]));table.style='Light Shading Accent 1'
            for row in value:
                for cell,text in zip(table.add_row().cells,row):
                    cell.text=str(text)
                    for paragraph in cell.paragraphs:
                        for run in paragraph.runs:run.font.size=Pt(9)
        else:
            p=doc.add_paragraph(('WHAT TO SAY: ' if kind=='say' else '')+value)
            if kind=='code':
                for run in p.runs:run.font.name='Courier New';run.font.size=Pt(9)
            if kind in ['say','note']:
                for run in p.runs:run.italic=True
sec.footer.paragraphs[0].text='INFLUENCE STUDIO | GUI EXPLANATION AND PRESENTER GUIDE | 21 NOVEMBER 2026'
doc.save(OUT/'GUI_Explanation_and_Presenter_Guide.docx')

styles=getSampleStyleSheet()
styles.add(ParagraphStyle(name='GTitle',fontName='Helvetica-Bold',fontSize=20,leading=24,textColor=colors.HexColor('#'+navy),spaceAfter=15))
styles.add(ParagraphStyle(name='GHead',fontName='Helvetica-Bold',fontSize=11,leading=14,textColor=colors.HexColor('#'+teal),spaceBefore=10,spaceAfter=6))
styles.add(ParagraphStyle(name='GBody',fontName='Helvetica',fontSize=9.2,leading=13,spaceAfter=9))
styles.add(ParagraphStyle(name='GCell',parent=styles['GBody'],fontSize=8,leading=10.5,spaceAfter=0))
styles.add(ParagraphStyle(name='GHeaderCell',parent=styles['GCell'],fontName='Helvetica-Bold',textColor=colors.white))
styles.add(ParagraphStyle(name='GQuote',parent=styles['GBody'],fontName='Helvetica-Oblique',backColor=colors.HexColor('#EAF5F0'),borderPadding=7,spaceBefore=5,spaceAfter=12))
styles.add(ParagraphStyle(name='GNote',parent=styles['GBody'],fontSize=8.1,leading=11,textColor=colors.HexColor('#'+gray)))
styles.add(ParagraphStyle(name='GCode',fontName='Courier',fontSize=8.5,leading=12,backColor=colors.HexColor('#F1F5F7'),borderPadding=7,spaceAfter=13))
def para(text,name='GBody'):return Paragraph(escape(str(text)),styles[name])
story=[]
for i,(title,blocks) in enumerate(pages):
    if i:story.append(PageBreak())
    story.append(para(f'{i+1:02d}  {title}','GTitle'))
    for kind,value in blocks:
        if kind=='h':story.append(para(value,'GHead'))
        elif kind=='code':story.append(Preformatted(value,styles['GCode']))
        elif kind=='table':
            columns=len(value[0]);width=A4[0]-88
            widths=[width*.31,width*.69] if columns==2 else ([width*.22,width*.36,width*.42] if columns==3 else [width/columns]*columns)
            table=Table([[para(cell,'GHeaderCell' if r==0 else 'GCell') for cell in row] for r,row in enumerate(value)],colWidths=widths,repeatRows=1)
            table.setStyle(TableStyle([('BACKGROUND',(0,0),(-1,0),colors.HexColor('#'+navy)),('ROWBACKGROUNDS',(0,1),(-1,-1),[colors.HexColor('#F1F6F6'),colors.white]),('VALIGN',(0,0),(-1,-1),'TOP'),('LEFTPADDING',(0,0),(-1,-1),7),('RIGHTPADDING',(0,0),(-1,-1),7),('TOPPADDING',(0,0),(-1,-1),6),('BOTTOMPADDING',(0,0),(-1,-1),6)]))
            story.extend([table,Spacer(1,9)])
        else:story.append(para(('WHAT TO SAY: ' if kind=='say' else '')+value, 'GQuote' if kind=='say' else 'GNote' if kind=='note' else 'GBody'))
def footer(canvas,document):
    canvas.setStrokeColor(colors.HexColor('#'+teal));canvas.line(44,43,A4[0]-44,43)
    canvas.setFont('Helvetica',8);canvas.setFillColor(colors.HexColor('#'+gray))
    canvas.drawString(44,29,'INFLUENCE STUDIO | GUI EXPLANATION AND PRESENTER GUIDE')
    canvas.drawRightString(A4[0]-44,29,str(document.page))
SimpleDocTemplate(str(OUT/'GUI_Explanation_and_Presenter_Guide.pdf'),pagesize=A4,leftMargin=44,rightMargin=44,topMargin=42,bottomMargin=58,title='Influence Studio: GUI Explanation and Presenter Guide',author='Project 15').build(story,onFirstPage=footer,onLaterPages=footer)
for name in ['GUI_Explanation_and_Presenter_Guide.docx','GUI_Explanation_and_Presenter_Guide.pdf']:
    p=OUT/name;print(p,p.stat().st_size,'bytes')
