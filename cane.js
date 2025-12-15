var pointm = [];
var ang;
var cuntA;
var lastMoveTime = 0;

function CubeBegin() {
    document.getElementById("ang").innerHTML = 90;
    document.getElementById("dist").innerHTML = 50;
    
    scene = new THREE.Scene();
    camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 5000);
    renderer = new THREE.WebGLRenderer({ antialias: true });
    renderer.shadowMap.enabled = true;
    renderer.shadowMap.type = THREE.PCFSoftShadowMap;
    camera.lookAt(scene.position);
    renderer.setSize(1200, 600);
    scene3d = document.getElementById("scene3d");
    scene3d.appendChild(renderer.domElement);
    
    const matrixp = [];
    for (let i = 0; i < 360; i++) {
        matrixp.push(new THREE.Vector3(1, 0, 0));
        var matrixgp = new THREE.BufferGeometry().setFromPoints(matrixp);
        var matrixgpm = new THREE.PointsMaterial({ color: 0x00ff00, size: 5 });
        pointm[i] = new THREE.Points(matrixgp, matrixgpm);
        scene.add(pointm[i]);
    }  

    const arrayA = [];
    arrayA.push(new THREE.Vector3(0, 0, 0));
    arrayA.push(new THREE.Vector3(1, 0, 0));
    var buffA = new THREE.BufferGeometry().setFromPoints(arrayA);
    var matA = new THREE.LineBasicMaterial({ color: 0xff0000 });
    lineA = new THREE.Line(buffA, matA);

    const discogeom = new THREE.BufferGeometry();
    const material2 = new THREE.MeshStandardMaterial({ color: 0xFE00FF });
    disco = new THREE.Mesh(discogeom, material2);
    disco.position.set(0, 0, 0);

    disco.add(lineA);
    scene.add(disco);
    scene.add(new THREE.AmbientLight(0xffffff));

    camera.position.set(400, 400, 400);

    controls = new THREE.OrbitControls(camera, renderer.domElement);
    render();

}

function render() {
    controls.update();
    disco.rotation.y = (document.getElementById("ang").innerHTML * Math.PI / 180);
    cuntA = parseFloat(document.getElementById("ang").innerHTML);
    pointm[cuntA].position.x = document.getElementById("dist").innerHTML * Math.cos(document.getElementById("ang").innerHTML * (Math.PI / 180));
    pointm[cuntA].position.z = document.getElementById("dist").innerHTML * Math.sin(-document.getElementById("ang").innerHTML * (Math.PI / 180));
    
    requestAnimationFrame(render);
    if (document.getElementById("dist").innerHTML > 0) {
        lineA.scale.x = lineA.scale.z = document.getElementById("dist").innerHTML;
    }

    const currentTime = Date.now();
    if (currentTime - lastMoveTime > document.getElementById('speedstep').value) {
        for (let i = 0; i < 360; i++) {
            pointm[i].position.x =0;  
            pointm[i].position.z =0;  
        }
        lastMoveTime = currentTime;
    }
    renderer.render(scene, camera);
}