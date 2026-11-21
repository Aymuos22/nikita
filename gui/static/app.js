'use strict';

const $ = selector => document.querySelector(selector);
const $$ = selector => [...document.querySelectorAll(selector)];
const fmt = value => new Intl.NumberFormat('en-IN').format(value);
const esc = value => String(value).replace(/[&<>"']/g, c => ({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
const palette = ['#269a83','#5d88c7','#ba89c2','#dc9970','#80a356','#789fa9','#d17d85','#a69bce'];
const color = community => palette[community % palette.length];
const state = { dataset:'sample', report:null, selected:null, busy:false, view:'overview', zoom:1, communityPage:0, rankingPage:0 };
const titles = {
    overview:['Your network, explained.','Discover the people, connections and communities that matter.','Overview'],
    communities:['Find your communities.','Explore the groups connected by fast, active relationships.','Communities'],
    campaign:['Make every connection count.','Plan a high-value creator campaign within your budget.','Campaign planner'],
    lab:['Explore the algorithms.','Compare strategies on the same network, text and constraints.','Algorithm lab'],
};

async function api(path, data) {
    const response = await fetch(path, data === undefined ? {} : {
        method:'POST', headers:{'Content-Type':'application/json'}, body:JSON.stringify(data),
    });
    const result = await response.json();
    if (!response.ok) throw new Error(result.error || 'Something went wrong. Please try again.');
    return result;
}

function message(text, error=false) {
    const element = $('#message');
    element.textContent = text;
    element.classList.toggle('error',error);
    element.hidden = !text;
}

function setBusy(busy) {
    state.busy = busy;
    document.body.classList.toggle('loading',busy);
    $('main').setAttribute('aria-busy',String(busy));
    $$('#run-analysis, #budget, [data-scenario], #data-dialog button, #data-dialog input').forEach(el => { el.disabled=busy; });
    $('#run-status').textContent = busy ? 'Running your analysis…' : state.report ? `Updated just now · ${fmt(state.report.elapsed_ms)} ms` : 'Ready to analyze';
}

function showView(view) {
    state.view = view;
    $$('.view-panel').forEach(el => { el.hidden = el.id !== `view-${view}`; });
    $$('.nav-item').forEach(el => {
        el.classList.toggle('active',el.dataset.view === view);
        if (el.dataset.view === view) el.setAttribute('aria-current','page');
        else el.removeAttribute('aria-current');
    });
    const [title,subtitle,breadcrumb] = titles[view];
    $('#page-title').textContent = title;
    $('#page-subtitle').textContent = subtitle;
    $('#breadcrumb').textContent = breadcrumb;
    if (view === 'communities') renderCommunities();
    if (view === 'campaign') renderCampaign();
}

function showDialog(id) {
    if (id === 'data-dialog') { $('#data-error').hidden=true; }
    const dialog = $('#'+id);
    if (!dialog.open) dialog.showModal();
}

async function runAnalysis(options={}) {
    if (state.busy) return false;
    const budget = options.budget ?? Number($('#budget').value);
    if ((options.budget===undefined && $('#budget').value.trim()==='') || !Number.isSafeInteger(budget) || budget<0 || budget>1000000) {
        message('Enter an integer campaign budget from 0 to 1,000,000.',true);
        return false;
    }
    const scenario = options.scenario ?? state.report?.scenario ?? 'normal';
    setBusy(true);
    message('');
    try {
        const result = await api('/api/analyze',{dataset:options.dataset ?? state.dataset,budget,scenario});
        state.report=result;
        state.dataset=result.dataset;
        state.selected=result.rankings[0]?.id ?? null;
        state.zoom=1;
        state.communityPage=0;
        state.rankingPage=0;
        $('#budget').value=result.budget;
        $('#graph-search').value='';
        $('#ranking-search').value='';
        $('#community-search').value='';
        render();
        if ($('#data-dialog').open) $('#data-dialog').close();
        return true;
    } catch(error) {
        if ($('#data-dialog').open) {
            $('#data-error').hidden=false;
            $('#data-error').textContent=error.message;
        } else message(error.message + (state.report ? ' Your last successful result is still shown.' : ''),true);
        return false;
    } finally {
        setBusy(false);
    }
}

function render() {
    const r=state.report;
    $('#dataset-name').textContent=r.dataset_name;
    $('#dataset-info').textContent=`${fmt(r.users)} users · ${fmt(r.relationships)} links`;
    $('#stat-users').textContent=fmt(r.users);
    $('#stat-links').textContent=`${fmt(r.active_relationships)} active relationships`;
    $('#stat-communities').textContent=fmt(r.communities);
    $('#community-nav').textContent=fmt(r.communities);
    $('#stat-campaign').textContent=fmt(r.campaign_value);
    $('#stat-selected').textContent=`${r.campaign_users.length} creators · ${r.campaign_cost} of ${r.budget} budget units`;
    $('#stat-flow').textContent=r.max_flow === null ? 'Not computed' : fmt(r.max_flow);
    $('#stat-flow-note').textContent=r.max_flow===null ? (r.users>2000 ? 'Flow limit: 2,000 users' : 'Requires distinct source and sink') : `User ${r.source_user} to user ${r.sink_user} · max flow`;
    $$('.segmented button').forEach(el => {
        const active=el.dataset.scenario===r.scenario;
        el.classList.toggle('selected',active);
        el.setAttribute('aria-pressed',String(active));
    });
    $('#export-open').disabled=false;
    $('#export-json').href=`/api/download?run=${r.run_id}&format=json`;
    $('#export-csv').href=`/api/download?run=${r.run_id}&format=csv`;
    $('#data-provenance').textContent=r.synthetic ? 'Synthetic data · fixed seed 15' : 'Imported CSV data · local processing';
    $('#route-source').value=r.source_user;
    $('#route-target').value=r.sink_user;
    $('#route-result').textContent=`Current scenario: ${r.scenario==='shock'?'capacity shock':'normal operation'}. Choose your endpoints.`;
    $('#route-result').classList.remove('error');
    $('#graph-count').textContent=`${r.graph_nodes.length} users shown`;
    $('#graph-empty').hidden=r.graph_nodes.length>0;
    $('#graph-disclosure').hidden=!(r.graph_sampled || r.graph_edges.length>=1000);
    $('#graph-disclosure').textContent=`Showing ${r.graph_nodes.length} of ${fmt(r.users)} users and up to 1,000 active links. Rankings, communities and paths use the full network.`;
    renderGraph();
    renderLeaderboard();
    renderSpotlight();
    renderPreview();
    renderCommunities();
    renderCampaign();
}

function avatar(name) {
    const clean=String(name).replace(/^user_/, 'U');
    return esc(clean.slice(0,3).toUpperCase());
}

function renderLeaderboard() {
    const r=state.report;
    const max=r.rankings[0]?.score || 1;
    $('#leaderboard').innerHTML=r.rankings.slice(0,6).map((user,i)=>`
        <button class="rank-row ${user.id===state.selected?'active':''}" data-user="${user.id}" aria-label="Inspect ${esc(user.name)}, influence score ${user.score}">
          <span class="rank-number">${String(i+1).padStart(2,'0')}</span>
          <span class="user-avatar">${avatar(user.name)}</span>
          <span><span class="user-name">${esc(user.name)}</span><span class="user-sub">Community ${user.community+1}</span></span>
          <span class="score-value">${fmt(user.score)}<span class="score-bar"><span data-bar="${Math.max(4,user.score/max*100)}"></span></span></span>
        </button>`).join('');
    $$('[data-bar]').forEach(el=>{ el.style.width=el.dataset.bar+'%'; });
    $$('#leaderboard [data-user]').forEach(el=>el.addEventListener('click',()=>selectUser(Number(el.dataset.user))));
}

function selectUser(id) {
    if (!state.report) return;
    state.selected=id;
    renderSpotlight();
    renderLeaderboard();
    highlightGraph();
}

function renderSpotlight() {
    const r=state.report;
    const user=r.rankings.find(u=>u.id===state.selected);
    if (!user) return;
    const selected=r.campaign_users.includes(user.id);
    $('#spotlight-tag').textContent=selected?'Campaign creator':`Community ${user.community+1}`;
    $('#spotlight-tag').classList.toggle('green',selected);
    $('#user-detail').innerHTML=`
      <div class="spotlight-head"><span class="user-avatar">${avatar(user.name)}</span><div><strong>${esc(user.name)}</strong><p>User ID ${user.id} · Community ${user.community+1}</p></div></div>
      <div class="detail-metrics"><div><strong>${fmt(user.score)}</strong><span>Influence score</span></div><div><strong>${user.incoming}</strong><span>Incoming links</span></div><div><strong>${fmt(user.engagement)}</strong><span>Interaction strength</span></div></div>
      <div class="path-label">Path from user ${r.source_user} · ${user.distance===null?'unreachable':user.distance+' delay units'}</div>
      <div class="path-chain">${user.path.length ? user.path.map(esc).join(' → ') : 'No active path from the source.'}</div>`;
}

function renderPreview() {
    const r=state.report;
    const selected=new Set(r.campaign_users);
    const creators=r.rankings.filter(user=>selected.has(user.id));
    $('#campaign-preview').innerHTML=`
      <div class="preview-value">${fmt(r.campaign_value)}<small>total influence score</small></div>
      <div class="budget-line"><span>${r.campaign_users.length} creators selected</span><span>${r.campaign_cost} / ${r.budget} units</span></div>
      <div class="progress-track"><span id="preview-progress"></span></div>
      <div class="selected-creators">${creators.slice(0,5).map(user=>`<span class="creator-chip">${esc(user.name)}</span>`).join('')}${creators.length>5?`<span class="creator-chip">+${creators.length-5} more</span>`:''}${!creators.length?'<span class="muted">No creators fit this campaign budget.</span>':''}</div>`;
    $('#preview-progress').style.width=(r.budget ? r.campaign_cost/r.budget*100 : 0)+'%';
}

function createSVG(tag,attributes={}) {
    const element=document.createElementNS('http://www.w3.org/2000/svg',tag);
    Object.entries(attributes).forEach(([key,value])=>element.setAttribute(key,String(value)));
    return element;
}

function layoutGraph(users) {
    const groups=[...new Set(users.map(u=>u.community))];
    const max=users[0]?.score || 1;
    const nodes=users.map((user,i)=>{
        const angle=i*2.39996323;
        const ring=users.length<20?115:125+50*Math.sin(i*1.73);
        const group=groups.indexOf(user.community);
        const ga=group/Math.max(groups.length,1)*Math.PI*2;
        const gx=groups.length>1?360+Math.cos(ga)*150:360;
        const gy=groups.length>1?205+Math.sin(ga)*85:205;
        return {...user,x:360+Math.cos(angle)*ring,y:205+Math.sin(angle)*ring*.8,gx,gy,radius:11+Math.sqrt(user.score/max)*13};
    });
    for(let step=0;step<150;step++) {
        for(let i=0;i<nodes.length;i++) {
            const a=nodes[i];
            let fx=(a.gx-a.x)*.008,fy=(a.gy-a.y)*.008;
            for(let j=0;j<nodes.length;j++) {
                if(i===j) continue;
                const b=nodes[j], dx=a.x-b.x,dy=a.y-b.y;
                const distance=Math.max(1,Math.hypot(dx,dy));
                const min=a.radius+b.radius+34;
                const force=distance<min?(min-distance)*.045:40/(distance*distance);
                fx+=dx/distance*force;
                fy+=dy/distance*force;
            }
            a.x=Math.min(665-a.radius,Math.max(55+a.radius,a.x+fx));
            a.y=Math.min(356-a.radius,Math.max(35+a.radius,a.y+fy));
        }
    }
    if (nodes.length>1 && nodes.length<30) {
        const xs=nodes.map(n=>n.x), ys=nodes.map(n=>n.y);
        const minX=Math.min(...xs),maxX=Math.max(...xs),minY=Math.min(...ys),maxY=Math.max(...ys);
        for(const node of nodes){
            node.x=100+(node.x-minX)/(maxX-minX || 1)*520;
            node.y=65+(node.y-minY)/(maxY-minY || 1)*260;
        }
    }
    return nodes;
}

function renderGraph() {
    const r=state.report;
    const svg=$('#network-graph');
    svg.replaceChildren();
    const defs=createSVG('defs');
    const marker=createSVG('marker',{id:'edge-arrow',viewBox:'0 0 8 8',refX:'7',refY:'4',markerWidth:'5',markerHeight:'5',orient:'auto-start-reverse'});
    marker.append(createSVG('path',{d:'M 0 0 L 8 4 L 0 8 z',fill:'#b2ceca',stroke:'none'}));
    defs.append(marker);svg.append(defs);
    const group=createSVG('g',{id:'graph-layer'});
    const nodes=layoutGraph(r.graph_nodes);
    const positions=new Map(nodes.map(n=>[n.id,n]));
    // Collapse parallel display lines to the lowest-delay active relationship.
    // Every input edge still contributes to the C++ analytics and counts.
    const displayedEdges=new Map();
    for(const edge of r.graph_edges){
        const key=edge.from+':'+edge.to;
        if(!displayedEdges.has(key)||edge.delay<displayedEdges.get(key).delay)displayedEdges.set(key,edge);
    }
    for (const edge of displayedEdges.values()) {
        const source=positions.get(edge.from),target=positions.get(edge.to);
        if (!source || !target || source.id===target.id) continue;
        const dx=target.x-source.x,dy=target.y-source.y,distance=Math.max(1,Math.hypot(dx,dy));
        const endX=target.x-dx/distance*(target.radius+4),endY=target.y-dy/distance*(target.radius+4);
        const line=createSVG('line',{x1:source.x,y1:source.y,x2:endX,y2:endY,class:'edge','marker-end':'url(#edge-arrow)','data-from':source.id,'data-to':target.id});
        const title=createSVG('title');
        title.textContent=`${source.name} → ${target.name} · delay ${edge.delay} · capacity ${edge.capacity}`;
        line.append(title);
        group.append(line);
    }
    for(const node of nodes) {
        const g=createSVG('g',{class:'node',transform:`translate(${node.x.toFixed(2)},${node.y.toFixed(2)})`,tabindex:'0',role:'button','aria-label':`${node.name}: score ${node.score}, community ${node.community+1}`,'data-user':node.id});
        g.append(createSVG('circle',{r:node.radius+6,class:'node-ring'}));
        g.append(createSVG('circle',{r:node.radius,class:'node-core',fill:color(node.community)}));
        const score=createSVG('text',{y:3.5,class:'node-score'});
        score.textContent=String(node.score);
        g.append(score);
        const label=createSVG('text',{y:node.radius+18,class:'node-label'});
        label.textContent=node.name.length>16?node.name.slice(0,14)+'…':node.name;
        g.append(label);
        const title=createSVG('title');
        title.textContent=`${node.name} · score ${node.score} = 10 × ${node.incoming} + ${node.engagement}`;
        g.append(title);
        g.addEventListener('click',()=>selectUser(node.id));
        g.addEventListener('keydown',event=>{if(event.key==='Enter' || event.key===' '){event.preventDefault();selectUser(node.id);}});
        group.append(g);
    }
    svg.append(group);
    const groups=[...new Set(nodes.map(u=>u.community))];
    $('#graph-legend').innerHTML=groups.slice(0,3).map(c=>`<span class="legend-item"><span class="legend-dot" data-color="${color(c)}"></span>Community ${c+1}</span>`).join('')+(groups.length>3?`<span>+${groups.length-3} groups</span>`:'');
    $$('[data-color]').forEach(el=>{el.style.backgroundColor=el.dataset.color;});
    applyZoom();
    highlightGraph();
}

function highlightGraph() {
    if (!state.report) return;
    const user=state.report.rankings.find(u=>u.id===state.selected);
    const pathEdges=new Set((user?.path||[]).slice(1).map((v,i)=>`${user.path[i]}:${v}`));
    const query=$('#graph-search').value.trim().toLowerCase();
    $$('#network-graph .node').forEach(el=>{
        const id=Number(el.dataset.user);
        const item=state.report.graph_nodes.find(u=>u.id===id);
        el.classList.toggle('selected',id===state.selected);
        el.classList.toggle('dim',Boolean(query)&&!item.name.toLowerCase().includes(query)&&!String(id).includes(query));
    });
    $$('#network-graph .edge').forEach(el=>{
        el.classList.toggle('highlight',pathEdges.has(`${el.dataset.from}:${el.dataset.to}`));
        el.classList.toggle('dim',Boolean(query));
    });
}

function applyZoom() {
    const layer=$('#graph-layer');
    if(layer) layer.setAttribute('transform',`translate(${360*(1-state.zoom)} ${205*(1-state.zoom)}) scale(${state.zoom})`);
}

function pageControls(element,page,total,onChange,size) {
    const pages=Math.max(1,Math.ceil(total/size));
    element.innerHTML=`<span>${total ? fmt(page*size+1)+'–'+fmt(Math.min(total,(page+1)*size)) : '0'} of ${fmt(total)}</span><span><button data-direction="-1" ${page<=0?'disabled':''}>Previous</button><button data-direction="1" ${page>=pages-1?'disabled':''}>Next</button></span>`;
    element.querySelectorAll('button').forEach(el=>el.addEventListener('click',()=>onChange(page+Number(el.dataset.direction))));
}

function renderCommunities() {
    if(!state.report) return;
    const query=$('#community-search').value.trim().toLowerCase();
    const map=new Map();
    for(const user of state.report.rankings) {
        if(!map.has(user.community)) map.set(user.community,[]);
        map.get(user.community).push(user);
    }
    const groups=[...map.entries()].sort((a,b)=>b[1].length-a[1].length||a[0]-b[0]).filter(([id,users])=>!query || users.some(u=>u.name.toLowerCase().includes(query)||String(u.id).includes(query)));
    const size=9;
    state.communityPage=Math.max(0,Math.min(state.communityPage,Math.ceil(groups.length/size)-1));
    $('#community-grid').innerHTML=groups.slice(state.communityPage*size,(state.communityPage+1)*size).map(([id,users])=>`
      <article class="community-card"><h3><span class="legend-dot" data-color="${color(id)}"></span>Community ${id+1}</h3><p>${fmt(users.length)} member${users.length===1?'':'s'} · ${fmt(users.reduce((s,u)=>s+u.score,0))} total score</p><div class="community-members">${users.slice(0,8).map(u=>`<button data-community-user="${u.id}">${esc(u.name)}</button>`).join('')}${users.length>8?`<span class="muted">+${fmt(users.length-8)} more</span>`:''}</div></article>`).join('') || '<div class="empty-state">No communities match your search.</div>';
    $$('#community-grid [data-color]').forEach(el=>{el.style.backgroundColor=el.dataset.color;});
    $$('[data-community-user]').forEach(el=>el.addEventListener('click',()=>{
        showView('overview');selectUser(Number(el.dataset.communityUser));$('#user-detail').scrollIntoView({behavior:'smooth',block:'center'});
    }));
    pageControls($('#community-pagination'),state.communityPage,groups.length,page=>{state.communityPage=page;renderCommunities();},size);
}

function renderCampaign() {
    if(!state.report) return;
    const r=state.report, selected=new Set(r.campaign_users);
    $('#candidate-description').textContent=`Exact integer optimization over ${r.campaign_candidate_count} of ${fmt(r.users)} users.`;
    $('#candidate-table').innerHTML=r.candidates.map(u=>`<tr><td><span class="user-name">${esc(u.name)}</span><span class="user-sub">User ID ${u.id}</span></td><td>${fmt(u.score)}</td><td>${u.cost} units</td><td>${selected.has(u.id)?'<span class="decision-selected">Selected</span>':'<span class="decision-not">Not selected</span>'}</td></tr>`).join('');
    $('#plan-summary').innerHTML=`<div class="plan-number">${fmt(r.campaign_value)} <small>influence score</small></div><div class="summary-row"><span>Selected creators</span><strong>${r.campaign_users.length}</strong></div><div class="summary-row"><span>Budget used</span><strong>${r.campaign_cost} / ${r.budget}</strong></div><div class="summary-row"><span>Budget remaining</span><strong>${r.budget-r.campaign_cost} units</strong></div><div class="summary-row"><span>Candidate pool</span><strong>Top ${r.campaign_candidate_count}</strong></div><div class="summary-row"><span>Strategy</span><strong>0/1 knapsack DP</strong></div>`;
    $('#topic-assignments').innerHTML=r.creator_topic_assignment.length?r.creator_topic_assignment.map(a=>`<div class="assignment"><span>User ${a.creator}</span><span>${esc(a.topic_name)}</span></div>`).join(''):`<p class="muted">${r.users>10000?'Matching is limited to 10,000 users.':'No eligible creator-topic pairs.'}</p>`;
}

function renderAllRankings() {
    if(!state.report) return;
    const query=$('#ranking-search').value.trim().toLowerCase();
    const ranks=state.report.rankings.map((u,i)=>({...u,rank:i+1})).filter(u=>u.name.toLowerCase().includes(query)||String(u.id).includes(query));
    const size=50;
    state.rankingPage=Math.max(0,Math.min(state.rankingPage,Math.ceil(ranks.length/size)-1));
    $('#all-ranking-rows').innerHTML=ranks.slice(state.rankingPage*size,(state.rankingPage+1)*size).map(u=>`<tr><td>${u.rank}</td><td><span class="user-name">${esc(u.name)}</span><span class="user-sub">ID ${u.id}</span></td><td>${fmt(u.score)}</td><td>${u.community+1}</td><td>${u.distance===null?'Unreachable':u.distance}</td></tr>`).join('') || '<tr><td colspan="5">No users match your search.</td></tr>';
    pageControls($('#ranking-pagination'),state.rankingPage,ranks.length,page=>{state.rankingPage=page;renderAllRankings();},size);
}

async function loadDataset(action,data) {
    if(state.busy) return;
    setBusy(true);
    $('#data-error').hidden=true;
    let dataset;
    try { dataset=await api(action,data); }
    catch(error) { $('#data-error').textContent=error.message;$('#data-error').hidden=false; }
    finally { setBusy(false); }
    if(dataset) await runAnalysis({dataset:dataset.dataset});
}

async function labAction(form,resultId,path,data) {
    const button=form.querySelector('button[type="submit"]');
    const output=$('#'+resultId);
    button.disabled=true;
    output.textContent='Running…';
    output.classList.remove('error');
    try {
        const result=await api(path,data);
        if(result.positions) output.textContent=`${fmt(result.positions.length)} match${result.positions.length===1?'':'es'} · ${result.elapsed_ms} ms\nByte offsets: ${result.positions.length?result.positions.slice(0,500).join(', '):'none'}${result.positions.length>500?' … (first 500 shown)':''}`;
        else output.textContent=`${result.output}\n${result.method} · ${result.scenario} · ${result.elapsed_ms} ms`;
    } catch(error) { output.textContent=error.message;output.classList.add('error'); }
    finally {button.disabled=false;}
}

function downloadText(text,name,type) {
    const url=URL.createObjectURL(new Blob([text],{type}));
    const a=document.createElement('a');a.href=url;a.download=name;a.click();
    setTimeout(()=>URL.revokeObjectURL(url),1000);
}

$$('.nav-item').forEach(el=>el.addEventListener('click',()=>showView(el.dataset.view)));
$('.brand').addEventListener('click',event=>{event.preventDefault();showView('overview');});
$('#change-data').addEventListener('click',()=>showDialog('data-dialog'));
// Keep dataset selection accessible in the compact mobile navigation.
const mobileData=document.createElement('button');
mobileData.className='button mobile-data-button';mobileData.textContent='Dataset';
mobileData.addEventListener('click',()=>showDialog('data-dialog'));
$('.topbar-right').prepend(mobileData);
$$('[data-close]').forEach(el=>el.addEventListener('click',()=>$('#'+el.dataset.close).close()));
$$('dialog').forEach(dialog=>dialog.addEventListener('click',event=>{
    if(event.target===dialog){const rect=dialog.getBoundingClientRect();if(event.clientX<rect.left||event.clientX>rect.right||event.clientY<rect.top||event.clientY>rect.bottom)dialog.close();}
}));
$('#run-analysis').addEventListener('click',()=>runAnalysis());
$('#budget').addEventListener('input',()=>{$('#run-status').textContent='Budget changed · run analysis to apply';});
$('#budget').addEventListener('keydown',event=>{if(event.key==='Enter')runAnalysis();});
$$('[data-scenario]').forEach(el=>el.addEventListener('click',()=>runAnalysis({scenario:el.dataset.scenario,budget:el.dataset.scenario==='shock'?10:35})));
$('#export-open').addEventListener('click',()=>showDialog('export-dialog'));
$('#load-sample').addEventListener('click',()=>runAnalysis({dataset:'sample',budget:35,scenario:'normal'}));
$('#generate-form').addEventListener('submit',event=>{event.preventDefault();loadDataset('/api/generate',{users:Number($('#generate-count').value)});});
$('#load-path-form').addEventListener('submit',event=>{event.preventDefault();loadDataset('/api/load',{path:$('#dataset-path').value});});
$('#csv-upload').addEventListener('change',async event=>{
    const files=[...event.target.files];
    const names=['users.csv','topics.csv','posts.csv','relationships.csv','interactions.csv'];
    if(files.length!==5 || names.some(name=>!files.some(f=>f.name===name)) || files.reduce((s,f)=>s+f.size,0)>12*1024*1024){
        $('#data-error').hidden=false;$('#data-error').textContent='Select exactly the five required CSV files, totaling no more than 12 MB.';event.target.value='';return;
    }
    try { const data=Object.fromEntries(await Promise.all(files.map(async file=>[file.name,await file.text()])));await loadDataset('/api/load',{files:data}); }
    catch(error){$('#data-error').hidden=false;$('#data-error').textContent=error.message;}
    event.target.value='';
});
$('#graph-search').addEventListener('input',highlightGraph);
$('#zoom-in').addEventListener('click',()=>{state.zoom=Math.min(2.5,state.zoom+.2);applyZoom();});
$('#zoom-out').addEventListener('click',()=>{state.zoom=Math.max(.5,state.zoom-.2);applyZoom();});
$('#zoom-reset').addEventListener('click',()=>{state.zoom=1;applyZoom();});
$('#all-rankings').addEventListener('click',()=>{renderAllRankings();showDialog('rankings-dialog');});
$('#ranking-search').addEventListener('input',()=>{state.rankingPage=0;renderAllRankings();});
$('#community-search').addEventListener('input',()=>{state.communityPage=0;renderCommunities();});
$('#open-campaign').addEventListener('click',()=>showView('campaign'));
$('#route-form').addEventListener('submit',event=>{
    event.preventDefault();if(!state.report)return;
    labAction(event.currentTarget,'route-result','/api/route',{dataset:state.dataset,scenario:state.report.scenario,source:Number($('#route-source').value),target:Number($('#route-target').value),method:$('#route-method').value});
});
$('#match-form').addEventListener('submit',event=>{
    event.preventDefault();labAction(event.currentTarget,'match-result','/api/match',{method:$('#match-method').value,text:$('#match-text').value,pattern:$('#match-pattern').value});
});
$('#run-workbench').addEventListener('click',async event=>{
    const button=event.currentTarget;button.disabled=true;$('#workbench-output').textContent='Running all 36 algorithm variants on the loaded dataset…';
    try {const result=await api('/api/workbench',{dataset:state.dataset,budget:state.report?.budget??35,scenario:state.report?.scenario??'normal'});$('#workbench-output').textContent=result.output+`\nCompleted in ${result.elapsed_ms} ms.`;}
    catch(error){$('#workbench-output').textContent=error.message;}
    finally{button.disabled=false;}
});
$('#run-benchmark').addEventListener('click',async event=>{
    const button=event.currentTarget;button.disabled=true;$('#workbench-output').textContent='Running small, medium, large and adversarial experiments across 36 variants. Each case uses three repetitions…';
    try {const result=await api('/api/benchmark',{});$('#workbench-output').textContent=`Completed in ${result.elapsed_ms} ms.\n${result.cases} cases across 36 variants. CSV downloaded.\n\nPer-call peak C++ heap payload is measured. Stack, malloc-only allocations and allocator metadata are excluded. Process RSS remains cumulative; working-set estimates are labeled separately.\n\n`+result.csv;downloadText(result.csv,'algorithm-benchmarks.csv','text/csv');}
    catch(error){$('#workbench-output').textContent=error.message;}
    finally{button.disabled=false;}
});


$('#run-comparison').addEventListener('click',async event=>{
    const button=event.currentTarget;button.disabled=true;
    $('#workbench-output').textContent='Comparing strategies on this dataset, budget and scenario…';
    try{
        const result=await api('/api/compare',{dataset:state.dataset,budget:state.report?.budget??35,scenario:state.report?.scenario??'normal'});
        const lines=result.rows.map(row=>`${row.algorithm.padEnd(26)} ${Number(row.mean_us).toFixed(2).padStart(12)} µs   ${fmt(Number(row.measured_peak_heap_bytes)).padStart(12)} heap bytes   value=${row.solution_value||row.quality_kind}   relative gap=${row.relative_gap||'n/a'}   ${row.quality_kind}`);
        $('#workbench-output').textContent=`${result.dataset_name} · ${result.scenario} · same inputs and candidate pool\nThree repetitions per strategy. Measured peak heap payload excludes stack and allocator overhead.\nExact feasible solutions have zero objective gap. Fractional values are upper bounds, not executable integer plans.\nDensity greedy is feasible but may lose value.\n\n`+lines.join('\n');
        downloadText(result.csv,'network-strategy-comparison.csv','text/csv');
    }catch(error){$('#workbench-output').textContent=error.message;}
    finally{button.disabled=false;}
});

(async function start(){
    try {
        const status=await api('/api/status');
        $('#engine-status').textContent=status.ready?'Analytics engine ready':'Build required';
        $('#engine-status').classList.toggle('ready',status.ready);
        await runAnalysis({budget:35,scenario:'normal'});
    } catch(error){message('Could not connect to the local engine. Start it with make gui. '+error.message,true);$('#engine-status').textContent='Engine unavailable';}
})();
