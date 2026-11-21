"""Generate presentation artifacts. Optional tools: python-docx, python-pptx, reportlab.
Run: python3 -m pip install python-docx python-pptx reportlab
     python3 scripts/build_presentation.py
These packages are not required by the C++ application.
"""
from pathlib import Path
import csv
from xml.sax.saxutils import escape

from docx import Document
from docx.shared import Inches as DInches, Pt as DPt, RGBColor as DColor
from pptx import Presentation
from pptx.util import Inches, Pt
from pptx.dml.color import RGBColor
from pptx.enum.shapes import MSO_SHAPE
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, PageBreak, Preformatted, Image as PDFImage
from reportlab.lib import colors
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.lib.enums import TA_LEFT
from reportlab.lib.pagesizes import A4
from reportlab.lib.utils import ImageReader

ROOT = Path(__file__).resolve().parents[1]
DEST = ROOT / 'docs/presentation'
DEST.mkdir(parents=True, exist_ok=True)
NAVY = '14293D'
TEAL = '087F8C'
LIGHT = 'EAF4F4'
GRAY = '536477'
TITLE = 'Social Network Influence and Community Analysis Tool'
rows = list(csv.DictReader((ROOT/'results/benchmarks.csv').open()))
bench = {(r['dataset'], r['algorithm']): float(r['mean_us']) for r in rows}

# Shared content keeps PDF and editable Word reports aligned.
pages = [
('Project overview and presentation plan', [
 ('lead', 'Project 15 | Design and Analysis of Algorithms | C++17 | Eight-week project brief'),
 ('p', 'An analytics application with the local Influence Studio browser GUI and a C++ command-line engine that represents users, posts, topics, relationships and interaction streams, then computes explainable rankings, propagation paths, communities and budget-constrained campaigns.'),
 ('h', 'What to say at the start'),
 ('quote', 'Our project turns a social network into a weighted and capacitated graph. It explains who ranks highly, how information can travel, which users form fast-link groups, and which creators can be selected within a campaign budget. We compare algorithms on identical inputs and verify their answers with independent tests.'),
 ('h', 'A 12-minute presentation'),
 ('table', [['Time','Topic','Evidence to show'], ['0:00-2:00','Problem, model and architecture','Slides 1-4'],['2:00-4:00','Algorithms, GUI and architecture','Slides 5-8'], ['4:00-8:00','Normal, shock and stress demo','GUI views and downloaded reports'], ['8:00-10:00','Correctness and performance','Slides 13-14 and test logs'], ['10:00-12:00','Limits, reduction and questions','Slides 15-16 and viva answers']]),
 ('h', 'Deliverables already available'),
 ('bullets', ['Reusable static library: build/libsocial.a and public headers in include/social/.', 'Influence Studio GUI, CLI/menu, sample CSV data, exports and 36 algorithm variants.', 'GUI/API/browser tests, benchmark runner, documentation and demo scripts. The original CLI baseline is tagged v1.0.0; the current acceptance release includes GUI and loaded-data comparisons.']),
 ('note', 'Presenter: add your name, roll number, class and actual individual algorithm ownership before submission. This report does not assert student authorship or human review.'),
]),
('System model and explainable outputs', [
 ('h', 'How data becomes a report'),
 ('pipeline', ['CSV inputs','Validation + ID index','Algorithms','CSV / JSON reports']),
 ('table', [['Record','Meaning'],['User','External ID, name and campaign cost'], ['Relationship','Directed edge: delay weight and independent capacity'], ['Post / topic','Authored text and topic eligibility'],['Interaction','From/to users, referenced post, timestamp and strength']]),
 ('h', 'Explain each output precisely'),
 ('bullets', ['Influence score = 10 x incoming relationship count + incoming interaction strength. Ties use ascending user ID.', 'Communities are weak components of positive-capacity links whose delay is at most 5.', 'Routes minimize total delay over active directed links; minimum delay is a propagation proxy, not a causal influence estimate.', 'Campaign DP maximizes additive score under the budget over at most the top 20 ranked users.', 'Maximum flow measures integer dissemination capacity from the first to last user.', 'Creator-topic matching allows one topic per creator and one creator per topic, using topics the creator has actually posted about.']),
 ('h', 'Important distinctions'),
 ('p', 'Delay and capacity have different roles. A link with zero capacity cannot carry propagation traffic. A shock changes available capacity; historical degree and engagement rankings stay unchanged. MST infrastructure uses the explicitly undirected projection.'),
 ('note', 'All supplied data is synthetic. Cost, delay, strength and capacity are educational scenario units.'),
]),
('Run the application: presenter runbook', [
 ('h', '1. Launch the graphical interface'),
 ('code', 'make gui'),
 ('p', 'Run from the Nikita repository directory. This builds the C++ application and opens Influence Studio at http://127.0.0.1:8765. Requirements: C++17 compiler, Make, Python 3 and a browser. No extra pip/npm packages are needed to use the GUI.'),
 ('h', '2. Walk through the interface'),
 ('table', [['View / control','What to demonstrate'], ['Overview','Sample loads automatically. Select a user/node; inspect score factors and path.'], ['Communities','Browse groups, search members, select a member.'], ['Campaign planner','Inspect costs, selected creators, total score and remaining budget.'], ['Algorithm lab','Compare routes/matchers, run the loaded-data workbench or compare this network.'], ['Change dataset / Export report','Load five CSV files or generate data; download the current JSON or CSV.']]),
 ('h', '3. Start, stop and recover'),
 ('p', 'Keep the terminal running. Ctrl+C stops the server and clears temporary dataset snapshots. On macOS, run_gui.command is a double-click launcher. Download reports before stopping if you want to keep them.'),
 ('code', 'python3 gui/server.py --port 8766 --no-browser'),
 ('p', 'Use an alternate port if 8765 is occupied. If Terminal reports getcwd: Operation not permitted, allow Terminal access to the Desktop folder in macOS privacy settings or use a checkout in a directory it can read.'),
 ('h', '4. CLI fallback'),
 ('code', './build/social_cli\n./build/social_cli demo results/presentation/demo'),
 ('note', 'The browser is a local interface; all assessed algorithms still execute in C++. The CLI and full CSV/JSON formats remain available.'),
]),
('Influence Studio: live dashboard walkthrough', [
 ('image', 'assets/GUI_Overview.png'),
 ('p', 'Actual sample screenshot: 12 users, one community, campaign 472 and flow 36. Browser requests pass through a local Python bridge to the C++ CLI and reusable library; the graph and leaderboard share that result.'),
 ('table', [['Action','What the audience should notice'], ['Select a node or ranked user','User spotlight explains the score and full source path.'], ['Change budget; Run analysis','The plan stays feasible and reflects the new budget.'], ['Switch to Capacity shock','Budget preset 10; flow 8; campaign score 265; two communities.'], ['Export report','JSON/CSV downloads are the original engine reports.']]),
 ('note', 'For large inputs the graph shows a disclosed subset, while full rankings, communities, campaign decisions and paths use the complete dataset. Original CSV files are not overwritten.'),
]),
('Live demonstration: expected scenario results', [
 ('table', [['Scenario','Users','Budget','Communities','Campaign','Max flow'], ['Normal','12','35','1','472','36'], ['Shock','12','10','2','265','8'],['Stress','10,000','35','1,401','1,233','Skipped']]),
 ('h', 'Normal operation: about 90 seconds'),
 ('p', 'GUI: choose the sample dataset and Normal operation. Inspect Overview and Campaign planner; select user_2; download a report. CLI fallback: ./build/social_cli analyze data/sample 35 results/presentation/normal'),
 ('quote', 'The ranking includes its factors. For example, user_2 scores 92: six incoming links contribute 60 and incoming interaction strength contributes 32. The campaign score is 472 within budget 35. Dijkstra and Bellman-Ford agree on path distances.'),
 ('h', 'Shock: about 60 seconds'),
 ('p', 'GUI: click Capacity shock. It applies budget 10 and reruns the same network. Open Communities and Campaign planner. CLI fallback: ./build/social_cli analyze data/sample 10 results/presentation/shock shock'),
 ('quote', 'Every capacity is divided by four with rounding down. The budget also falls to 10. Maximum flow falls from 36 to 8; two fast-link communities remain. Exact campaign value is 265, while the fractional upper bound is 270.6. The historical ranking remains unchanged.'),
 ('h', 'Stress: about 60 seconds'),
 ('p', 'GUI: switch back to Normal operation first (budget 35), then Change dataset > Generate a network > 10000 > Generate & analyze. CLI fallback: ./build/social_cli demo results/presentation/demo'),
 ('p', 'The CLI fallback runs all three scenarios and the syllabus workbench; the GUI generates the stress network directly. Export the resulting JSON or inspect results/presentation/demo/stress.json after the CLI run. All 10,000 users are ranked and have route/community results. Campaign selection is exact within the top-20 pool. Dense flow is deliberately skipped above 2,000 vertices and exported as null.'),
 ('note', 'Expected values use the bundled generator with seed 15. Elapsed times vary by machine and run. Changed data or seeds may change scenario values.'),
]),
('Compare algorithmic strategies on identical inputs', [
 ('h', 'Routing comparison'),
 ('p', 'In Algorithm lab, enter source 0 and target 11 and select each routing strategy. The currently analyzed scenario is respected: normal delay is 6; shock delay is 23. Compare strategies inside one scenario.'),
 ('code', './build/social_cli route data/sample 0 11 dijkstra\n./build/social_cli route data/sample 0 11 bellman\n./build/social_cli route data/sample 0 11 floyd'),
 ('p', 'All three return delay 6. Dijkstra and Bellman-Ford show the path 0 -> 5 -> 10 -> 11. Dijkstra requires nonnegative weights; Bellman-Ford supports negative analytical weights and detects reachable negative cycles. Floyd-Warshall computes all-pairs distances on bounded graphs.'),
 ('h', 'Text matching comparison'),
 ('p', 'In Algorithm lab, set text to aaaaa and pattern to aaa; switch between all four matcher strategies and click Find matches.'),
 ('code', "./build/social_cli match naive 'aaaaa' aaa\n./build/social_cli match rk 'aaaaa' aaa\n./build/social_cli match fa 'aaaaa' aaa\n./build/social_cli match kmp 'aaaaa' aaa"),
 ('p', 'Each returns zero-based byte offsets 0, 1 and 2, including overlaps. KMP is linear in text plus pattern size. Rabin-Karp verifies hash hits, so collisions cannot introduce false matches.'),
 ('h', 'Campaign, memory and quality comparison'),
 ('p', 'Choose Compare this network in Algorithm lab to download elapsed time, measured peak heap, exact reference and relative-gap fields for the identical active input.'),
 ('code', './build/social_cli workbench data/sample 35\nmake comparisons'),
 ('p', 'The loaded sample uses its real candidate costs and scores. In the controlled three-creator comparison, exact DP/search value is 220, feasible density greedy is 160, and fractional value 240 is an upper bound rather than an executable plan. The loaded-data workbench also shows Classical/Strassen equality, loaded-text Huffman roundtrip and every required module with explicit bounds. N-Queens remains a validation exercise.'),
 ('note', 'Compare algorithms inside the same scenario and candidate pool. Normal and shock are different inputs, so differences between their objective values are not algorithm-quality comparisons.'),
]),
('Algorithm coverage and design choices', [
 ('table', [['Area','Required implementations'], ['Foundations / divide and conquer','Merge sort, quicksort, both binary searches, maximum subarray, powering, celebrity, Classical and Strassen products'], ['Greedy','Activity selection, interval partitioning, fractional knapsack, Huffman, Dijkstra, Prim and Kruskal'], ['Dynamic programming','0/1 knapsack, matrix-chain order, LCS, optimal BST, Bellman-Ford, Floyd-Warshall, Held-Karp TSP'], ['Search','N-Queens, sum of subsets, knapsack backtracking, FIFO/LIFO/least-cost branch-and-bound'], ['Capacity / assignment','Ford-Fulkerson and maximum bipartite matching'], ['Number / text','CRT, naive, Rabin-Karp, finite-automaton and KMP matching']]),
 ('h', 'Useful complexities to explain'),
 ('table', [['Algorithm','Time bound','Reason to use it'], ['Merge sort','Theta(n log n)','Stable deterministic rankings'], ['Three-way quicksort','Expected n log n; worst n^2','Good duplicate handling'],['Dijkstra','O((V+E) log(E+1))','Nonnegative single-source delay'], ['Bellman-Ford','O(VE+V)','Signed costs, negative-cycle checks'], ['Knapsack DP','Theta(k(B+1))','Exact moderate integer budgets'], ['Held-Karp TSP','O(k^2 2^k)','Small exact outreach tours'], ['KMP','Theta(n+m)','Linear text search']]),
 ('h', 'One recurrence to derive aloud'),
 ('p', 'Merge sort: T(n) = 2T(n/2) + Theta(n). Each recursion level costs Theta(n), and there are Theta(log n) levels, so T(n) = Theta(n log n). Strassen uses seven half-size products and has Theta(n^log2(7)) time.'),
 ('note', 'The complete API mapping, recurrence derivations and proof arguments are in docs/ALGORITHMS.md and docs/COMPLEXITY.md. Validation exercises are distinguished from integrated product features.'),
]),
('Verification and benchmark evidence', [
 ('h', 'Executed correctness evidence'),
 ('table', [['Check','Recorded result'], ['Unit and oracle tests','15,265 passed'], ['Existing CLI integration checks','48 passed'], ['Additional independent core end-to-end assertions','60,678 passed'], ['GUI HTTP integration tests','8 grouped tests passed'], ['Chromium GUI workflows / responsive layout','64 browser checks passed'], ['Benchmark experiments','144 cases across 36 variants; 3 repetitions each'], ['Address / undefined-behavior sanitizers','No errors reported on executed paths'], ['External static-library consumer','Linked and ran successfully']]),
 ('p', 'Core checks recomputed rankings, communities, paths, campaigns and small-network flows. GUI checks cover scenario controls, CSV imports, downloads, graph interaction, all route/matcher strategies, 10,000-user rendering and mobile layout.'),
 ('h', 'Commands to show evidence'),
 ('code', 'make test\nmake benchmark\nmake comparisons'),
 ('h', 'Stored instrumented benchmark: mean microseconds'),
 ('table', [['Input','Dijkstra','Bellman-Ford'], ['Random, 10,000 vertices',f"{bench['large','dijkstra']:.2f}",f"{bench['large','bellman_ford']:.2f}"], ['Adversarial, 2,048 vertices',f"{bench['adversarial','dijkstra']:.2f}",f"{bench['adversarial','bellman_ford']:.2f}"]]),
 ('p', 'The adversarial chain orders edges against propagation, forcing repeated Bellman-Ford passes. Favorable random edge order can let Bellman-Ford beat Dijkstra in measured time despite the latter having a better worst-case guarantee. These numbers are stored observations, not live timing promises.'),
 ('note', 'Per-call C++ heap payload is measured separately; stack, input buffers allocated earlier, allocator metadata and malloc-only allocation are excluded. RSS remains cumulative. Timings include meter overhead. See results/comparisons/REPORT.md.'),
]),
('Viva answers, limits and presentation recovery', [
 ('h', 'Five short answers'),
 ('bullets', ['Why not only count followers? The score includes incoming interaction strength as well as degree and exports both factors; it remains a transparent heuristic.', 'Why is fractional knapsack not the campaign answer? Creator contracts are indivisible; fractional purchases only bound the best achievable integer score.', 'Why is campaign decision NP-complete in the general model? Reduce SUBSET SUM: set cost_i = value_i = a_i and B = K = T. A selected sum must be both at most and at least T, hence exactly T. A chosen subset is polynomially verifiable.', 'Why is DP pseudo-polynomial? O(kB) depends on numeric B, while binary input uses only O(log B) bits. Fixed top-20 program instances are a restricted problem.', 'What certifies maximum flow? No residual source-to-sink path remains; the source-side residual cut has capacity equal to the reported flow.']),
 ('h', 'State these practical limits'),
 ('p', 'Campaign optimality covers at most 20 candidates and an additive score, not deduplicated audience reach. Communities use a fixed delay threshold and weak connectivity. Dense max-flow is skipped above 2,000 vertices; creator-topic matching is skipped above 10,000 users. Other exact solvers enforce their own size limits. The GUI caps imports at 10,000 users / 12 MB and samples large visual graphs. No platform data is scraped and no content is posted.'),
 ('h', 'If something interrupts the live demo'),
 ('table', [['Issue','Action'], ['GUI not running','Run make gui; open http://127.0.0.1:8765.'], ['CSV file cannot open','Use data/sample from the root; confirm all five files exist.'], ['Oversized exact instance','Use the bundled sample or reduce budget/instance size.'], ['Port occupied / Desktop denied','Use --port 8766; grant Terminal Desktop access or relocate checkout.'], ['Need a known output','Use results/DEMO_OUTPUT.txt and results/benchmarks.csv.'], ['Need a full rerun','Run sh scripts/demo.sh; it rewrites default demo/benchmark files.']]),
 ('note', 'Keep sanitizer rebuilds outside the timed presentation. For a development check use make sanitize, then make clean and make -j4 to restore optimized binaries. Human ownership and peer-review evidence remain presenter-provided.'),
]),
]

# PDF report: exactly one authored section per page.
styles = getSampleStyleSheet()
styles.add(ParagraphStyle(name='ReportTitle', fontName='Helvetica-Bold', fontSize=21, leading=25, textColor=colors.HexColor('#'+NAVY), spaceAfter=15))
styles.add(ParagraphStyle(name='SectionHead', fontName='Helvetica-Bold', fontSize=11, leading=14, textColor=colors.HexColor('#'+TEAL), spaceBefore=10, spaceAfter=5))
styles.add(ParagraphStyle(name='Copy', fontName='Helvetica', fontSize=9.3, leading=13.2, spaceAfter=7))
styles.add(ParagraphStyle(name='NoteText', parent=styles['Copy'], fontSize=8.3, leading=11, textColor=colors.HexColor('#'+GRAY), backColor=colors.HexColor('#'+LIGHT), borderPadding=7, spaceBefore=7))
styles.add(ParagraphStyle(name='Cell', parent=styles['Copy'], fontSize=8, leading=10.5, spaceAfter=0))
styles.add(ParagraphStyle(name='CodeText', fontName='Courier', fontSize=8.2, leading=11.5, backColor=colors.HexColor('#F0F3F6'), borderPadding=7, spaceBefore=5, spaceAfter=9))
def para(text, style='Copy'):
    return Paragraph(escape(text), styles[style])
styles.add(ParagraphStyle(name='HeaderCell', parent=styles['Cell'], textColor=colors.white, fontName='Helvetica-Bold'))
def pdf_table(data):
    width=A4[0]-88
    columns=len(data[0])
    if columns==2: widths=[width*.29,width*.71]
    elif columns==3: widths=[width*.28,width*.32,width*.40]
    else: widths=[width/columns]*columns
    table=Table([[para(str(c),'HeaderCell' if row_index==0 else 'Cell') for c in row] for row_index,row in enumerate(data)], colWidths=widths, hAlign='LEFT', repeatRows=1)
    table.setStyle(TableStyle([('BACKGROUND',(0,0),(-1,0),colors.HexColor('#'+NAVY)),('TEXTCOLOR',(0,0),(-1,0),colors.white),('ROWBACKGROUNDS',(0,1),(-1,-1),[colors.HexColor('#F1F6F7'),colors.white]),('VALIGN',(0,0),(-1,-1),'TOP'),('LEFTPADDING',(0,0),(-1,-1),7),('RIGHTPADDING',(0,0),(-1,-1),7),('TOPPADDING',(0,0),(-1,-1),7),('BOTTOMPADDING',(0,0),(-1,-1),7)]))
    return table
story=[]
for index,(heading,blocks) in enumerate(pages):
    if index: story.append(PageBreak())
    story.append(para(f'{index+1:02d}  {heading}', 'ReportTitle'))
    for kind,value in blocks:
        if kind=='h': story.append(para(value,'SectionHead'))
        elif kind=='code': story.append(Preformatted(value, styles['CodeText']))
        elif kind=='image':
            path=DEST/value; image_width,image_height=ImageReader(str(path)).getSize(); width=A4[0]-88
            story.extend([PDFImage(str(path),width=width,height=width*image_height/image_width),Spacer(1,12)])
        elif kind=='table': story.extend([pdf_table(value),Spacer(1,7)])
        elif kind=='bullets':
            for item in value: story.append(para('- '+item))
        elif kind=='pipeline': story.append(pdf_table([value]))
        else: story.append(para(value, 'NoteText' if kind=='note' else 'Copy'))
def footer(canvas, doc):
    canvas.setStrokeColor(colors.HexColor('#'+TEAL));canvas.setLineWidth(1)
    canvas.line(44,43,A4[0]-44,43)
    canvas.setFont('Helvetica',8);canvas.setFillColor(colors.HexColor('#'+GRAY))
    canvas.drawString(44,29,'PROJECT 15  |  RUN AND PRESENTATION REPORT  |  21 NOV 2026')
    canvas.drawRightString(A4[0]-44,29,str(doc.page))
SimpleDocTemplate(str(DEST/'Project_15_Presentation_Report.pdf'),pagesize=A4,rightMargin=44,leftMargin=44,topMargin=42,bottomMargin=57,title=TITLE,author='Project 15').build(story,onFirstPage=footer,onLaterPages=footer)

# Word version: editable content with code blocks and tables.
doc=Document()
section=doc.sections[0];section.top_margin=DInches(.65);section.bottom_margin=DInches(.65)
section.left_margin=DInches(.65);section.right_margin=DInches(.65)
normal=doc.styles['Normal'];normal.font.name='Calibri';normal.font.size=DPt(10)
normal.paragraph_format.space_after=DPt(6)
for style in ['Title','Heading 1','Heading 2']:
    doc.styles[style].font.color.rgb=DColor.from_string(NAVY if style!='Heading 2' else TEAL)
for index,(heading,blocks) in enumerate(pages):
    if index:doc.add_page_break()
    doc.add_heading(f'{index+1:02d}  {heading}',0)
    for kind,value in blocks:
        if kind=='h':doc.add_heading(value,2)
        elif kind=='table' or kind=='pipeline':
            values=value if kind=='table' else [value]
            table=doc.add_table(rows=0,cols=len(values[0]));table.style='Light Shading Accent 1'
            for values_row in values:
                for cell,text in zip(table.add_row().cells,values_row):cell.text=str(text)
        elif kind=='image':doc.add_picture(str(DEST/value),width=DInches(6.5))
        elif kind=='code':
            p=doc.add_paragraph();r=p.add_run(value);r.font.name='Courier New';r.font.size=DPt(8)
        elif kind=='bullets':
            for text in value:doc.add_paragraph(text,style='List Bullet')
        else:
            p=doc.add_paragraph(value)
            if kind in ('quote','note'):
                for run in p.runs:run.italic=True
    section.footer.paragraphs[0].text='PROJECT 15 | Run and presentation report | 21 November 2026'
doc.save(DEST/'Project_15_Presentation_Report.docx')

# Presentation: large readable type, sparse content, complete presenter notes.
prs=Presentation();prs.slide_width=Inches(13.333);prs.slide_height=Inches(7.5)

def box(slide,x,y,w,h,color,rounded=False):
    shape=slide.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE if rounded else MSO_SHAPE.RECTANGLE,Inches(x),Inches(y),Inches(w),Inches(h))
    shape.fill.solid();shape.fill.fore_color.rgb=RGBColor.from_string(color);shape.line.fill.background()
    return shape

def text(slide,x,y,w,h,value,size=22,color=NAVY,bold=False,font='Aptos'):
    shape=slide.shapes.add_textbox(Inches(x),Inches(y),Inches(w),Inches(h))
    frame=shape.text_frame;frame.word_wrap=True
    for i,line in enumerate(value.split('\n')):
        p=frame.paragraphs[0] if i==0 else frame.add_paragraph()
        p.text=line;p.font.name=font;p.font.size=Pt(size);p.font.bold=bold;p.font.color.rgb=RGBColor.from_string(color);p.space_after=Pt(12)
    return shape

def slide(title,kicker='PROJECT 15',notes=''):
    s=prs.slides.add_slide(prs.slide_layouts[6]);s.background.fill.solid();s.background.fill.fore_color.rgb=RGBColor(255,255,255)
    box(s,0,0,.16,7.5,TEAL)
    text(s,.55,.32,12,.3,kicker,11,TEAL,True)
    text(s,.55,.82,12.1,1.15,title,32,NAVY,True)
    box(s,.55,6.99,12.2,.015,'CBD9DF')
    text(s,.55,7.1,11,.25,'INFLUENCE STUDIO  |  GUI + CLI  |  C++17 ENGINE',9,GRAY)
    text(s,12.1,7.08,.6,.25,str(len(prs.slides)),10,GRAY)
    if notes:s.notes_slide.notes_text_frame.text=notes
    return s

def bullets(s,items,y=2.15,size=23):
    for i,item in enumerate(items):
        text(s,.75,y+i*.82,.3,.45,'•',size,TEAL,True)
        text(s,1.12,y+i*.82,11.4,.75,item,size)

def command(s,value,y=4.9,h=1):
    box(s,.7,y,11.9,h,'EFF4F6',True)
    text(s,.95,y+.15,11.4,h-.18,value,16,NAVY,font='Courier New')

def cards(s,entries,y=2.35):
    w=11.9/len(entries)
    for i,(metric,label) in enumerate(entries):
        x=.7+i*w
        box(s,x,y,w-.16,1.6,LIGHT,True)
        text(s,x+.18,y+.19,w-.52,.65,metric,34,TEAL,True)
        text(s,x+.18,y+.94,w-.52,.5,label,16,GRAY)

s=slide('Social Network Influence\nand Community Analysis Tool','DESIGN AND ANALYSIS OF ALGORITHMS',pages[0][1][3][1])
text(s,.75,3.05,11.7,1.1,'Explainable rankings. Constrained campaigns.\nVerified algorithm comparisons.',27,TEAL)
text(s,.75,5.2,11.8,.7,'Project 15 • C++17 • Eight-week brief\nPresenter: add your name / roll number',18,GRAY)

s=slide('What problem does the tool solve?',notes='Explain that this is a synthetic-data educational analytics product. It does not claim causal influence or guaranteed real-world reach.')
bullets(s,['Who ranks highly, and why?','How can a message travel with the least delay?','Which users form fast-link communities?','Which creators fit a campaign budget?','What happens when capacity and budget fall?'])

s=slide('From records to decisions',notes='Relationships are directed and have separate delay and capacity fields. The ID index maps external IDs to compact graph vertices. Algorithms are separated from persistence and presentation.')
for i,(heading,body) in enumerate([('INPUT','Users, posts, topics\nlinks, interactions'),('VALIDATE','CSV schema\nIDs and constraints'),('ANALYZE','Graphs, DP, sorting\nflow and matching'),('EXPLAIN','Score factors\npaths, CSV / JSON')]):
    x=.72+i*3.12
    box(s,x,2.65,2.86,2.2,LIGHT,True)
    text(s,x+.2,2.94,2.5,.5,heading,21,TEAL,True)
    text(s,x+.2,3.65,2.5,.9,body,17)
    if i<3:text(s,x+2.89,3.25,.3,.4,'›',25,TEAL,True)
text(s,.8,5.4,11.8,.8,'One C++ library powers the GUI, CLI, loaded-data workbench and comparisons.',23)

s=slide('Every output has an explicit model',notes='Rank = 10 times incoming degree plus incoming strength. Communities use weak connectivity after filtering delay <=5 and capacity >0. Campaign exactness is limited to the top 20 candidates.')
bullets(s,['Rank = 10 × incoming links + incoming interaction strength','Communities = weak components of active links with delay ≤ 5','Paths = minimum total delay over active directed edges','Campaign = maximum additive score under an integer budget','Assignment = one eligible creator per topic'],size=21)

s=slide('A complete 36-variant algorithm workbench',notes='Do not claim every algorithm is part of integrated analytics. N-Queens, celebrity and some other modules are executable syllabus exercises. Use docs/ALGORITHMS.md and docs/REQUIREMENTS.md for exact mappings and bounds. The workbench now uses the selected dataset.')
for i,(heading,body) in enumerate([('DIVIDE & CONQUER','Sort/search, maximum subarray,\npowering, celebrity, matrix products'),('GREEDY','Scheduling, Huffman, fractional\nallocation, shortest paths, MST'),('DYNAMIC PROGRAMMING','Knapsack, chains, LCS, OBST,\nBellman-Ford, Floyd, TSP'),('EXACT SEARCH','N-Queens, subsets, backtracking,\nFIFO / LIFO / least-cost B&B'),('FLOW & MATCHING','Ford-Fulkerson capacity model,\ncreator-topic bipartite assignment'),('NUMBER & TEXT','CRT, naive, Rabin-Karp,\nfinite automaton and KMP')]):
    col=i%2;row=i//2;x=.75+col*6.15;y=2.1+row*1.5
    text(s,x,y,5.7,.35,heading,15,TEAL,True)
    text(s,x,y+.4,5.65,.9,body,18)

s=slide('Start Influence Studio',notes='From the Nikita repository run make gui. It builds the C++ engine, starts a Python-standard-library loopback server and opens the browser. Keep Terminal running; Ctrl+C stops the server. GUI imports need no pip/npm packages. If macOS denies Desktop access, allow it for Terminal or relocate the checkout. CLI fallback: ./build/social_cli.')
command(s,'make gui',y=2.1,h=.9)
text(s,.9,3.4,11.7,.65,'Open http://127.0.0.1:8765',26,TEAL,True)
text(s,.9,4.3,11.7,1.8,'The sample network loads automatically.\nExplore the graph, communities, campaign and algorithm lab.\nKeep the terminal open; Ctrl+C stops the GUI.',22)

s=slide('Meet Influence Studio',notes='This is a real screenshot of the sample network. Explain that the graph is interactive and node size reflects score. Click user_2 to inspect the formula. Use Communities, Campaign planner and Algorithm lab for the next views. Data source and report export are explicit controls.')
s.shapes.add_picture(str(DEST/'assets/GUI_Overview.png'),Inches(.65),Inches(2.05),width=Inches(6.85))
for i,(heading,body) in enumerate([('OVERVIEW','Graph, rankings and score factors'),('COMMUNITIES','Searchable fast-link groups'),('CAMPAIGN PLANNER','Candidate costs and decisions'),('ALGORITHM LAB','Loaded-data workbench and comparisons')]):
    y=2.2+i*1.1
    text(s,7.95,y,4.7,.35,heading,16,TEAL,True)
    text(s,7.95,y+.38,4.7,.6,body,19)

s=slide('The GUI keeps the C++ engine at the center',notes='Browser HTML/CSS/JavaScript sends local JSON requests to the standard-library Python HTTP bridge. The bridge invokes the existing C++ CLI with argument arrays; it does not reimplement assessed algorithms. Reports are the engine original CSV/JSON format. Data and generated results are temporary unless downloaded.')
for i,(heading,body) in enumerate([('BROWSER','HTML / CSS / JavaScript\nInteractive views'),('PYTHON BRIDGE','Local HTTP + snapshots\nSubprocess calls'),('C++ CLI','Validation + orchestration\nScenario reports'),('ALGORITHM LIBRARY','Student implementations\nShared testable interfaces')]):
    x=.72+i*3.12
    box(s,x,2.65,2.86,2.2,LIGHT,True)
    text(s,x+.2,2.94,2.5,.5,heading,17,TEAL,True)
    text(s,x+.2,3.65,2.5,.9,body,16)
text(s,.85,5.5,11.7,.9,'Local processing. No external platform API.\nOne algorithm implementation, two ways to use it.',24,TEAL)

s=slide('Demo 1: normal operation',notes='Read score factors, show user_2 = 10*6 + 32 = 92. Explain candidate feasibility and show JSON. Timings vary; route objectives agree, not necessarily every tied route.')
cards(s,[('12','Users'),('35','Budget'),('472','Campaign score'),('36','Maximum flow')])
text(s,.9,4.35,11.7,.6,'One fast-link community. Explainable ranking factors. Exact strategy agreement.',21)
text(s,.9,5.25,11.7,1.05,'Overview → Normal operation → inspect user_2 → Export report',22,TEAL)

s=slide('Demo 2: capacity and budget shock',notes='Same data, capacities floor(c/4), budget 10. Historical ranking is unchanged. Zero-capacity links disappear from propagation and community construction. Fractional bound 270.6 exceeds legal integer optimum 265.')
cards(s,[('10','Budget'),('265','Campaign score'),('8','Maximum flow'),('2','Communities')])
text(s,.9,4.35,11.7,.6,'Capacity ÷ 4, rounded down; budget falls from 35 to 10.',23)
text(s,.9,5.25,11.7,1.05,'Select Capacity shock. Inspect Communities and Campaign planner.',22,TEAL)

s=slide('Demo 3: 10,000-user stress case',notes='GUI: restore Normal operation (budget 35), then Change dataset, generate 10000 users. The CLI demo command alternatively runs normal, shock and stress, followed by the workbench. Stress has 1,401 communities and campaign score 1,233 for seed 15. All exported stress users were independently checked. Flow is null by documented cap.')
cards(s,[('10,000','Users and posts'),('1,401','Communities'),('1,233','Campaign score')])
text(s,.9,4.35,11.7,.9,'Campaign exactness: top 20 candidates.\nDense flow skipped above 2,000 users; exported as null.',22)
text(s,.9,5.65,11.7,.85,'Normal operation → Change dataset → Generate 10,000 users',22,TEAL)

s=slide('Same input. Different strategies. Same answer.',notes='Use Algorithm lab for route and text strategy selectors. In normal operation the route commands return delay 6; after shock the delay is 23. Compare only within the same scenario; path is 0 -> 5 -> 10 -> 11. Matching aaaaa against aaa returns 0,1,2 for every matcher. Campaign 220 versus fractional 240 is the workbench fixture, not the normal network fixture.')
bullets(s,['Routing: Dijkstra / Bellman-Ford / Floyd → delay 6','Text: naive / Rabin-Karp / DFA / KMP → offsets 0, 1, 2','Campaign: DP / backtracking / all B&B variants → 220','Fractional campaign relaxation → upper bound 240','Classical and Strassen → identical matrix product'],y=2.15,size=21)

s=slide('Engine and interface verified separately',notes='The current audit records 15265 unit/oracle checks, 48 CLI integration checks and 60678 additional assertions; current sanitizer evidence is in results/ACCEPTANCE_SANITIZER_OUTPUT.txt; four direct heap-meter invariants also pass. The GUI adds eight grouped HTTP tests and 64 Chromium workflow/layout checks, including network comparison downloads. This is execution evidence, not proof of absence of all defects. See results/GUI_VALIDATION.txt for the current GUI record.')
cards(s,[('15,265','Unit / oracle checks'),('48','CLI integration checks'),('64','Browser checks')])
bullets(s,['8 grouped HTTP tests exercise the real C++ bridge','60,678 independent core E2E assertions in the recorded audit','Desktop/mobile, uploads, downloads, algorithms and stress views'],y=4.25,size=20)

s=slide('Time, memory and solution quality',notes='Use recorded means from results/benchmarks.csv. All alternatives receive identical inputs within each comparison. Reverse edge ordering makes the adversarial chain expensive for Bellman-Ford. RSS is cumulative. A separate per-call new/new[] payload meter measures algorithm heap; stack and allocator overhead are excluded. Compare this network and make comparisons record exact references, feasible heuristic quality and fractional bounds. The controlled case is exact 220 versus feasible greedy 160 and fractional bound 240.')
text(s,.85,2.15,11.7,.6,'144 cases • 36 variants • 3 repetitions • time + per-call heap',23,TEAL,True)
text(s,.85,3.12,11.7,1.8,
     'Random graph, 10,000 vertices\n'+f"Dijkstra: {bench['large','dijkstra']:.1f} µs     Bellman-Ford: {bench['large','bellman_ford']:.1f} µs\n"+
     'Reverse-order chain, 2,048 vertices\n'+f"Dijkstra: {bench['adversarial','dijkstra']:.1f} µs     Bellman-Ford: {bench['adversarial','bellman_ford']:.1f} µs",22)
text(s,.85,5.6,11.7,.8,'Controlled campaign: exact 220 · feasible greedy 160 · fractional bound 240',22,TEAL)

s=slide('What can we claim—and what remains limited?',notes='The general campaign decision is weakly NP-complete: cost=value=a_i, budget=threshold=T reduces SUBSET SUM. This is a claim about the unbounded mathematical problem, not the fixed top-20 implementation. Explain synthetic data, additive score and weak components honestly.')
bullets(s,['Exact campaign score within the disclosed top-20 pool','Transparent heuristic influence; no causal reach guarantee','Threshold communities; no modularity optimization','Bounded exact solvers and dense-flow size limit','Local GUI: 10,000 users / 12 MB; large graph display sampled'],size=21)

s=slide('Ready for the live demonstration',notes='Run the commands from the repository root. The complete PDF/Word report contains the detailed runbook, expected outputs, recurrence, reduction and viva answers. Add real individual ownership information before academic submission.')
command(s,'make gui\n# Verification in another terminal:\nmake test',2.25,1.8)
text(s,.9,4.65,11.7,1.1,'Explain the model → run the scenarios → show agreement\n→ discuss evidence and limits.',26,TEAL,True)
text(s,.9,6.0,11.7,.4,'Questions and viva',21,GRAY)
prs.save(DEST/'Project_15_Presentation.pptx')

run_sheet='''PROJECT 15 - LIVE RUN SHEET
From a terminal:

cd /Users/soumyadarshan/Desktop/Nikita
make -j4

# Recommended: graphical interface (keep this terminal open)
make gui
# Browser: http://127.0.0.1:8765
# Ctrl+C stops the server. For an alternate port:
# python3 gui/server.py --port 8766 --no-browser

# GUI demo flow:
# 1. Sample loads: campaign 472, flow 36, one community.
# 2. Select a graph node; inspect its score and path.
# 3. Communities and Campaign planner show groups and budget decisions.
# 4. Capacity shock: budget 10, campaign 265, flow 8, two communities.
# 5. Restore Normal operation BEFORE generating the stress dataset.
# 6. Change dataset > Generate a network > 10000 > Generate & analyze.
# 7. Inspect the disclosed graph sample and full searchable rankings.
# 8. Algorithm lab: routes 0 to 11; all methods give normal delay 6.
#    On the shocked sample all methods give delay 23.
# 9. Match aaaaa / aaa with every method: offsets 0, 1, 2.
# 10. Run the loaded-data workbench and Compare this network; export JSON / CSV.
# GUI datasets/results are temporary; download reports you want to keep.

# Alternative CLI menu (stop GUI first or use a second terminal)
./build/social_cli
# Enter: 1, data/sample, 3, 35, results/presentation/normal, 0

# Reproducible scenarios
./build/social_cli analyze data/sample 35 results/presentation/normal
./build/social_cli analyze data/sample 10 results/presentation/shock shock
./build/social_cli demo results/presentation/demo

# Same-input routing comparisons (all delay 6)
./build/social_cli route data/sample 0 11 dijkstra
./build/social_cli route data/sample 0 11 bellman
./build/social_cli route data/sample 0 11 floyd

# Same-input text comparisons (all offsets 0, 1, 2)
./build/social_cli match naive 'aaaaa' aaa
./build/social_cli match rk 'aaaaa' aaa
./build/social_cli match fa 'aaaaa' aaa
./build/social_cli match kmp 'aaaaa' aaa

# Evidence and report
make test
make test-gui
make comparisons
./build/social_cli workbench data/sample 35
./build/social_bench results/presentation/benchmarks.csv
python3 -m json.tool results/presentation/normal.json

EXPECTED: normal campaign 472 / flow 36 / communities 1;
shock campaign 265 / flow 8 / communities 2;
stress 10000 users / campaign 1233 / communities 1401 / flow skipped.

Presentation files:
Project_15_Presentation.pptx - slides with presenter notes
Project_15_Presentation_Report.pdf - printable report
Project_15_Presentation_Report.docx - editable report

Source of evidence: results/benchmarks.csv, results/VALIDATION.txt,
results/GUI_VALIDATION.txt, the local results/test-e2e/REPORT.txt (when retained),
and docs/COMPLEXITY.md.
On macOS, getcwd: Operation not permitted means Terminal needs Desktop access;
allow it in privacy settings or move the checkout to an accessible directory.
Sanitizers are optional development checks: make sanitize.
Restore optimized build afterward: make clean; make -j4.
'''
(DEST/'LIVE_RUN_SHEET.txt').write_text(run_sheet)
for path in sorted(DEST.iterdir()):
    if path.is_file(): print(path.relative_to(ROOT),path.stat().st_size,'bytes')
