"""
SIMULACIÓN INDUSTRIAL - Línea de Ensamble de Laptops
Generado por Yori v5.0 (Versión Mejorada para Demo)

Características:
- Simulación de eventos discretos con colas realistas
- Múltiples réplicas con análisis estadístico
- Identificación de cuellos de botella
- Análisis de costos y optimización
- Visualización de resultados
"""

import random
from statistics import mean, stdev
from collections import deque
from dataclasses import dataclass, field
from typing import List, Dict, Tuple
import time

# ============================================
# CONFIGURACIÓN
# ============================================

@dataclass
class StationConfig:
    """Configuración de una estación de trabajo"""
    name: str
    capacity: int
    time_mean: float  # segundos
    time_std: float   # segundos
    cost_per_hour: float
    failure_rate: float = 0.0  # probabilidad de falla por operación
    repair_time: float = 0.0   # tiempo de reparación en segundos

# Configuración de estaciones (del DSL original)
STATIONS_CONFIG = {
    'motherboard': StationConfig('Motherboard', 1, 20, 3, 15),
    'ram': StationConfig('RAM', 1, 15, 2, 15),
    'disco': StationConfig('Disco', 1, 25, 5, 15, failure_rate=0.002, repair_time=1800),
    'carcasa': StationConfig('Carcasa', 2, 30, 4, 12),
    'pruebas': StationConfig('Pruebas', 1, 60, 10, 20)
}

# Parámetros de simulación
ARRIVAL_RATE = 45  # segundos entre llegadas
TOTAL_LAPTOPS = 500
SIM_TIME_HOURS = 8
SIM_TIME_SECONDS = SIM_TIME_HOURS * 3600
REPLICAS = 10
BUFFER_CAPACITY = 50
DEFECT_RATE = 0.05  # 5% de defectos en pruebas
SEED = 42

# ============================================
# CLASES DE SIMULACIÓN
# ============================================

@dataclass
class Laptop:
    """Representa una laptop en proceso"""
    id: int
    arrival_time: float
    start_times: Dict[str, float] = field(default_factory=dict)
    end_times: Dict[str, float] = field(default_factory=dict)
    is_defective: bool = False
    completion_time: float = 0.0

class Station:
    """Estación de trabajo con cola y capacidad"""
    def __init__(self, config: StationConfig):
        self.config = config
        self.queue = deque()
        self.in_process = []
        self.total_processed = 0
        self.total_wait_time = 0.0
        self.busy_time = 0.0
        self.is_failed = False
        self.failure_end_time = 0.0
        
    def is_available(self, current_time: float) -> bool:
        """Verifica si la estación está disponible"""
        if self.is_failed and current_time < self.failure_end_time:
            return False
        return len(self.in_process) < self.config.capacity
    
    def add_to_queue(self, laptop: Laptop, current_time: float):
        """Agrega laptop a la cola"""
        laptop.start_times[self.config.name] = current_time
        self.queue.append(laptop)
    
    def start_processing(self, current_time: float) -> Tuple[Laptop, float]:
        """Inicia procesamiento de siguiente laptop en cola"""
        if not self.queue or not self.is_available(current_time):
            return None, 0.0
        
        laptop = self.queue.popleft()
        
        # Tiempo de espera
        wait_time = current_time - laptop.start_times[self.config.name]
        self.total_wait_time += wait_time
        
        # Tiempo de procesamiento (distribución normal)
        process_time = max(1, random.normalvariate(
            self.config.time_mean, 
            self.config.time_std
        ))
        
        # Simular falla ocasional
        if random.random() < self.config.failure_rate:
            self.is_failed = True
            self.failure_end_time = current_time + self.config.repair_time
            return None, 0.0
        
        self.in_process.append((laptop, current_time + process_time))
        self.busy_time += process_time
        
        return laptop, process_time
    
    def finish_processing(self, current_time: float) -> List[Laptop]:
        """Finaliza procesamiento de laptops completadas"""
        finished = []
        remaining = []
        
        for laptop, end_time in self.in_process:
            if current_time >= end_time:
                laptop.end_times[self.config.name] = current_time
                self.total_processed += 1
                finished.append(laptop)
            else:
                remaining.append((laptop, end_time))
        
        self.in_process = remaining
        return finished
    
    def get_utilization(self, total_time: float) -> float:
        """Calcula utilización de la estación"""
        max_capacity_time = self.config.capacity * total_time
        return (self.busy_time / max_capacity_time * 100) if max_capacity_time > 0 else 0
    
    def get_avg_wait_time(self) -> float:
        """Calcula tiempo promedio de espera"""
        return self.total_wait_time / self.total_processed if self.total_processed > 0 else 0

class ProductionLine:
    """Línea completa de producción"""
    def __init__(self):
        self.stations = {
            name: Station(config) 
            for name, config in STATIONS_CONFIG.items()
        }
        self.buffer = deque(maxlen=BUFFER_CAPACITY)
        self.completed = []
        self.defective = []
        self.wip_samples = []
        self.laptop_counter = 0
    
    def create_laptop(self, current_time: float) -> Laptop:
        """Crea nueva laptop"""
        self.laptop_counter += 1
        return Laptop(id=self.laptop_counter, arrival_time=current_time)
    
    def get_wip(self) -> int:
        """Calcula Work In Process actual"""
        wip = len(self.buffer)
        for station in self.stations.values():
            wip += len(station.queue) + len(station.in_process)
        return wip
    
    def sample_wip(self):
        """Registra muestra de WIP"""
        self.wip_samples.append(self.get_wip())

# ============================================
# MOTOR DE SIMULACIÓN
# ============================================

def simulate_production(replica_num: int = 0) -> Dict:
    """Ejecuta una réplica de la simulación"""
    random.seed(SEED + replica_num)
    
    line = ProductionLine()
    current_time = 0.0
    next_arrival = ARRIVAL_RATE
    laptops_to_arrive = TOTAL_LAPTOPS
    
    # Timeline de eventos
    events = []
    
    print(f"\n  📦 Réplica {replica_num + 1}: Simulando {SIM_TIME_HOURS} horas...", end=" ")
    
    while current_time < SIM_TIME_SECONDS or line.get_wip() > 0:
        # Evento: llegada de nueva laptop
        if laptops_to_arrive > 0 and current_time >= next_arrival:
            laptop = line.create_laptop(current_time)
            line.stations['motherboard'].add_to_queue(laptop, current_time)
            next_arrival += ARRIVAL_RATE
            laptops_to_arrive -= 1
        
        # Procesar cada estación
        station_names = list(STATIONS_CONFIG.keys())
        
        for i, station_name in enumerate(station_names):
            station = line.stations[station_name]
            
            # Finalizar laptops en proceso
            finished = station.finish_processing(current_time)
            
            # Mover a siguiente etapa
            for laptop in finished:
                if i < len(station_names) - 1:
                    # Siguiente estación
                    next_station_name = station_names[i + 1]
                    
                    # Etapa especial: después de motherboard va a buffer
                    if station_name == 'motherboard':
                        line.buffer.append(laptop)
                    elif station_name == 'ram':
                        line.stations[next_station_name].add_to_queue(laptop, current_time)
                    else:
                        line.stations[next_station_name].add_to_queue(laptop, current_time)
                else:
                    # Última estación (pruebas)
                    if random.random() < DEFECT_RATE:
                        laptop.is_defective = True
                        line.defective.append(laptop)
                    else:
                        laptop.completion_time = current_time
                        line.completed.append(laptop)
            
            # Iniciar procesamiento de laptops en cola
            if station_name == 'ram' and line.buffer:
                # RAM toma del buffer
                laptop = line.buffer.popleft()
                station.add_to_queue(laptop, current_time)
            
            station.start_processing(current_time)
        
        # Muestrear WIP cada minuto
        if int(current_time) % 60 == 0:
            line.sample_wip()
        
        # Avanzar tiempo (simulación de tiempo discreto)
        current_time += 1
        
        # Timeout de seguridad
        if current_time > SIM_TIME_SECONDS * 2:
            break
    
    print("✓")
    
    # Calcular métricas
    cycle_times = [
        laptop.completion_time - laptop.arrival_time 
        for laptop in line.completed
    ]
    
    return {
        'completed': len(line.completed),
        'defective': len(line.defective),
        'throughput': len(line.completed) / SIM_TIME_HOURS,
        'avg_cycle_time': mean(cycle_times) / 60 if cycle_times else 0,  # minutos
        'max_wip': max(line.wip_samples) if line.wip_samples else 0,
        'avg_wip': mean(line.wip_samples) if line.wip_samples else 0,
        'stations': {
            name: {
                'utilization': station.get_utilization(SIM_TIME_SECONDS),
                'avg_wait': station.get_avg_wait_time() / 60,  # minutos
                'processed': station.total_processed
            }
            for name, station in line.stations.items()
        }
    }

# ============================================
# ANÁLISIS Y VISUALIZACIÓN
# ============================================

def analyze_results(results: List[Dict]):
    """Analiza resultados de múltiples réplicas"""
    
    print("\n" + "="*70)
    print(" "*15 + "🏭 RESULTADOS DE SIMULACIÓN 🏭")
    print(" "*10 + "Línea de Ensamble de Laptops - TechMex")
    print("="*70)
    
    # Throughput
    throughputs = [r['throughput'] for r in results]
    print(f"\n📊 THROUGHPUT (Producción):")
    print(f"   Promedio:     {mean(throughputs):>8.2f} unidades/hora")
    print(f"   Desv. Est:    {stdev(throughputs):>8.2f}")
    print(f"   Mínimo:       {min(throughputs):>8.2f} unidades/hora")
    print(f"   Máximo:       {max(throughputs):>8.2f} unidades/hora")
    
    # Producción total
    total_completed = sum(r['completed'] for r in results)
    total_defective = sum(r['defective'] for r in results)
    print(f"\n✅ PRODUCCIÓN TOTAL ({REPLICAS} réplicas):")
    print(f"   Completadas:  {total_completed:>8} unidades")
    print(f"   Defectuosas:  {total_defective:>8} unidades")
    print(f"   Tasa defect:  {(total_defective/(total_completed+total_defective)*100):>8.2f}%")
    
    # Tiempo de ciclo
    cycle_times = [r['avg_cycle_time'] for r in results]
    print(f"\n⏱️  TIEMPO DE CICLO:")
    print(f"   Promedio:     {mean(cycle_times):>8.2f} minutos")
    print(f"   Mínimo:       {min(cycle_times):>8.2f} minutos")
    print(f"   Máximo:       {max(cycle_times):>8.2f} minutos")
    
    # WIP
    avg_wips = [r['avg_wip'] for r in results]
    max_wips = [r['max_wip'] for r in results]
    print(f"\n📦 WORK IN PROCESS (WIP):")
    print(f"   Promedio:     {mean(avg_wips):>8.2f} unidades")
    print(f"   Máximo obs:   {max(max_wips):>8.0f} unidades")
    
    # Verificar Ley de Little
    throughput_per_sec = mean(throughputs) / 3600
    little_cycle = mean(avg_wips) / throughput_per_sec / 60
    print(f"\n📐 VERIFICACIÓN - LEY DE LITTLE:")
    print(f"   WIP = Throughput × Tiempo_Ciclo")
    print(f"   {mean(avg_wips):.2f} ≈ {mean(throughputs):.2f}/hr × {mean(cycle_times):.2f} min")
    print(f"   Tiempo ciclo calculado: {little_cycle:.2f} min")
    print(f"   Diferencia: {abs(little_cycle - mean(cycle_times)):.2f} min ({'✓ OK' if abs(little_cycle - mean(cycle_times)) < 1 else '⚠ Check'})")
    
    # Utilización por estación
    print(f"\n🔧 UTILIZACIÓN DE ESTACIONES:")
    print(f"   {'Estación':<15} {'Utiliz %':>10} {'Espera (min)':>15} {'Procesadas':>12}")
    print(f"   {'-'*15} {'-'*10} {'-'*15} {'-'*12}")
    
    station_utils = {}
    station_waits = {}
    
    for station_name in STATIONS_CONFIG.keys():
        utils = [r['stations'][station_name]['utilization'] for r in results]
        waits = [r['stations'][station_name]['avg_wait'] for r in results]
        processed = [r['stations'][station_name]['processed'] for r in results]
        
        station_utils[station_name] = mean(utils)
        station_waits[station_name] = mean(waits)
        
        print(f"   {station_name.capitalize():<15} {mean(utils):>9.1f}% {mean(waits):>14.2f} {int(mean(processed)):>12}")
    
    # Identificar cuello de botella
    bottleneck = max(station_utils, key=station_utils.get)
    print(f"\n🚨 CUELLO DE BOTELLA IDENTIFICADO:")
    print(f"   Estación:     {bottleneck.upper()}")
    print(f"   Utilización:  {station_utils[bottleneck]:.1f}%")
    print(f"   Espera prom:  {station_waits[bottleneck]:.2f} minutos")
    
    # Sugerencias
    print(f"\n💡 SUGERENCIAS DE MEJORA:")
    if station_utils[bottleneck] > 85:
        config = STATIONS_CONFIG[bottleneck]
        print(f"   ⚠️  Alta utilización detectada en {bottleneck}")
        print(f"   → Opción 1: Agregar {config.capacity} estación(es) más")
        print(f"      Impacto estimado: +{(100 - station_utils[bottleneck])/2:.1f}% throughput")
        print(f"   → Opción 2: Reducir tiempo proceso en {config.time_mean*0.1:.0f} seg")
        print(f"      Impacto estimado: +{station_utils[bottleneck]*0.1:.1f}% throughput")
    
    # Costo estimado
    total_hours = SIM_TIME_HOURS * REPLICAS
    total_cost = sum(
        config.cost_per_hour * config.capacity * total_hours
        for config in STATIONS_CONFIG.values()
    )
    cost_per_unit = total_cost / total_completed if total_completed > 0 else 0
    
    print(f"\n💰 ANÁLISIS DE COSTOS:")
    print(f"   Costo operación total: ${total_cost:,.2f} USD")
    print(f"   Costo por unidad:      ${cost_per_unit:,.2f} USD")
    print(f"   Horas-máquina totales: {total_hours:.0f} hrs")
    
    print("\n" + "="*70)
    print("✅ Análisis completado")
    print("="*70 + "\n")
    
    return {
        'bottleneck': bottleneck,
        'avg_throughput': mean(throughputs),
        'avg_cycle_time': mean(cycle_times)
    }

# ============================================
# MAIN - EJECUCIÓN
# ============================================

def main():
    """Función principal"""
    print("\n" + "="*70)
    print(" "*20 + "🚀 YORI SIMULATION ENGINE 🚀")
    print(" "*15 + "Simulación de Eventos Discretos")
    print("="*70)
    
    print(f"\n⚙️  CONFIGURACIÓN:")
    print(f"   Duración:     {SIM_TIME_HOURS} horas por réplica")
    print(f"   Réplicas:     {REPLICAS}")
    print(f"   Laptops:      {TOTAL_LAPTOPS} unidades")
    print(f"   Estaciones:   {len(STATIONS_CONFIG)}")
    
    print(f"\n🏭 ESTACIONES CONFIGURADAS:")
    for name, config in STATIONS_CONFIG.items():
        print(f"   • {config.name:<12} Cap: {config.capacity}  Tiempo: {config.time_mean}±{config.time_std}s")
    
    print(f"\n▶️  Iniciando simulación...")
    
    start = time.time()
    results = []
    
    for i in range(REPLICAS):
        result = simulate_production(i)
        results.append(result)
    
    elapsed = time.time() - start
    
    print(f"\n⏱️  Simulación completada en {elapsed:.2f} segundos")
    
    # Analizar y mostrar resultados
    summary = analyze_results(results)
    
    print(f"💾 Datos listos para exportación/análisis adicional")
    print(f"🎯 Siguiente paso: Optimización basada en cuello de botella ({summary['bottleneck']})\n")

if __name__ == "__main__":
    main()