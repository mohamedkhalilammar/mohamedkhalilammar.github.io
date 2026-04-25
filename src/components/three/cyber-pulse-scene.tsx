"use client";

import { Canvas, useFrame } from "@react-three/fiber";
import { Float, Line, Points, PointMaterial } from "@react-three/drei";
import { useRef } from "react";
import type { Group } from "three";

function createDeterministicPoints(count: number): Float32Array {
  let seed = 1731;
  const values: number[] = [];

  const next = (): number => {
    seed = (seed * 48271) % 2147483647;
    return seed / 2147483647;
  };

  for (let index = 0; index < count; index += 1) {
    const radius = 5 + next() * 6;
    const theta = next() * Math.PI * 2;
    const phi = Math.acos(2 * next() - 1);

    values.push(
      radius * Math.sin(phi) * Math.cos(theta),
      radius * Math.sin(phi) * Math.sin(theta),
      radius * Math.cos(phi),
    );
  }

  return new Float32Array(values);
}

const DUST_POINTS = createDeterministicPoints(900);

function ThreatCore() {
  const groupRef = useRef<Group>(null);

  useFrame((_, delta) => {
    if (!groupRef.current) {
      return;
    }

    groupRef.current.rotation.y += delta * 0.2;
    groupRef.current.rotation.x += delta * 0.08;
  });

  return (
    <Float speed={1.8} rotationIntensity={0.4} floatIntensity={0.6}>
      <group ref={groupRef}>
        <mesh>
          <icosahedronGeometry args={[1.35, 0]} />
          <meshStandardMaterial
            color="#00F0FF"
            emissive="#D900FF"
            emissiveIntensity={0.8}
            wireframe
          />
        </mesh>
        <Line
          points={[
            [-1.8, -0.6, 0.2],
            [1.8, 0.7, -0.1],
          ]}
          color="#D900FF"
          lineWidth={2}
          transparent
          opacity={0.9}
        />
      </group>
    </Float>
  );
}

function PulseDust() {
  return (
    <Points positions={DUST_POINTS} stride={3} frustumCulled>
      <PointMaterial color="#38bdf8" size={0.035} sizeAttenuation depthWrite={false} transparent opacity={0.5} />
    </Points>
  );
}

export function CyberPulseScene() {
  return (
    <div className="scene-wrapper" aria-hidden style={{ position: "absolute", top: 0, left: 0, right: 0, bottom: 0, zIndex: 0, opacity: 0.7, pointerEvents: "none" }}>
      <Canvas camera={{ position: [0, 0, 6], fov: 50 }}>
        <ambientLight intensity={0.4} />
        <pointLight color="#00F0FF" position={[3, 2, 4]} intensity={25} />
        <pointLight color="#D900FF" position={[-3, -2, -4]} intensity={18} />
        <ThreatCore />
        <PulseDust />
      </Canvas>
    </div>
  );
}
