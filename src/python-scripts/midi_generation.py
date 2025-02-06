import json
from basic_pitch.inference import predict
from basic_pitch import ICASSP_2022_MODEL_PATH
import sys

# Get the audio path from the command line arguments
audio_path = sys.argv[1]

# Call the model to get predictions
model_output, midi_data, note_events = predict(audio_path)

note_events_json = []
for item in note_events:
    if len(item) != 5:
        print(f"Unexpected structure: {item}")  # Debugging print
        continue  # Skip unexpected items

    event = {
        "start_time": float(item[0]),  # Ensure it's a Python float
        "end_time": float(item[1]),
        "midi_pitch": int(item[2]),  # Ensure it's a Python int
        "confidence": float(item[3]),
        "amplitudes": [int(a) for a in item[4]]  # Convert each amplitude to Python int
    }
    note_events_json.append(event)
# Save the note_events as JSON
with open("note_events.json", "w") as f:
    json.dump(note_events_json, f, indent=4)

# Save the note_events as JSON
with open("note_events.json", "w") as f:
    json.dump(note_events_json, f, indent=4)

midi_data.write("output.mid")

print("note_events saved as note_events.json")
